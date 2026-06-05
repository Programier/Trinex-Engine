#include <UI/backend.hpp>

#include <Core/base_engine.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/templates.hpp>
#include <Core/math/math.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/viewport_client.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Input/event_system.hpp>
#include <Input/input_codes.hpp>
#include <Input/input_events.hpp>
#include <Platform/platform.hpp>
#include <RHI/context.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Trinex::UI::Backend
{
	namespace RenderBackend
	{
		class ImGuiPipeline : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(ImGuiPipeline, GlobalGraphicsPipeline);

			const RHIShaderParameterInfo* m_texture_parameter    = nullptr;
			const RHIShaderParameterInfo* m_projection_parameter = nullptr;

			RHITexture* m_texture = nullptr;
			RHISampler* m_sampler = nullptr;
			RHIResourcePtr<RHIBuffer> m_projection;


		public:
			void bind(RHIContext* ctx, const Matrix4f& projection)
			{
				ctx->barrier(m_projection.get(), RHIAccess::TransferDst);
				ctx->update(m_projection.get(), &projection, {.size = sizeof(projection)});
				ctx->barrier(m_projection.get(), RHIAccess::UniformBuffer);
				ctx->bind_uniform_buffer(m_projection.get(), m_projection_parameter->binding);
			}

			void bind(RHIContext* ctx, RHITexture* texture)
			{
				if (m_texture != texture)
				{
					ctx->bind_srv(texture->as_srv(), m_texture_parameter->binding);
					m_texture = texture;
				}
			}

			void bind(RHIContext* ctx, RHISampler* sampler)
			{
				if (m_sampler != sampler)
				{
					ctx->bind_sampler(sampler, m_texture_parameter->binding);
					m_sampler = sampler;
				}
			}

			void setup(RHIContext* ctx)
			{
				m_texture = nullptr;
				m_sampler = nullptr;

				ctx->bind_pipeline(rhi_pipeline());
				ctx->depth_stencil_state(RHIDepthStencilState());
				ctx->blending_state(RHIBlendingState::translucent());
				ctx->rasterizer_state(RHIRasterizerState());
			}
		};

		trinex_implement_pipeline(ImGuiPipeline, "[shaders]:/TrinexEditor/imgui.slang")
		{
			m_texture_parameter    = find_parameter("texture");
			m_projection_parameter = find_parameter("projection");

			constexpr auto flags = RHIBufferFlags::UniformBuffer | RHIBufferFlags::TransferDst;
			m_projection         = RHI::instance()->create_buffer(sizeof(Matrix4f), flags);
		}

		struct ImGuiTrinexData {
			RHIContext* context     = nullptr;
			RHITexture* target      = nullptr;
			Texture2D* font_texture = nullptr;
		};

		struct ImGuiTrinexViewportData {
			RHIResourcePtr<RHIBuffer> vertex_buffer;
			RHIResourcePtr<RHIBuffer> index_buffer;
			u64 vertex_count = 0;
			u64 index_count  = 0;
		};

		static ImGuiTrinexData* backend_data()
		{
			return ImGui::GetCurrentContext() ? (ImGuiTrinexData*) ImGui::GetIO().BackendRendererUserData : nullptr;
		}

		// RENDERING FUNCTIONS

		static void setup_render_state(ImDrawData* draw_data)
		{
			auto bd       = backend_data();
			auto pipeline = ImGuiPipeline::instance();

			bd->context->viewport(RHIViewport());

			float L = draw_data->DisplayPos.x;
			float R = L + draw_data->DisplaySize.x;
			float T = draw_data->DisplayPos.y;
			float B = T + draw_data->DisplaySize.y;

			pipeline->setup(bd->context);
			pipeline->bind(bd->context, Math::ortho(L, R, B, T, 0.f, 1.f));
		}

		static void destroy_texture(ImTextureData* tex)
		{
			RHITexture* texture = static_cast<RHITexture*>(tex->BackendUserData);
			texture->release();

			tex->BackendUserData = nullptr;
			tex->SetTexID(ImTextureID_Invalid);
			tex->SetStatus(ImTextureStatus_Destroyed);
		}

		static void update_texture(RHIContext* ctx, ImTextureData* tex)
		{
			auto bd = backend_data();

			if (tex->Status == ImTextureStatus_WantCreate)
			{
				trinex_assert(tex->TexID == ImTextureID_Invalid && tex->BackendUserData == nullptr);
				trinex_assert(tex->Format == ImTextureFormat_RGBA32);
				unsigned int* pixels = (unsigned int*) tex->GetPixels();

				auto rhi             = RHI::instance();
				constexpr auto flags = RHITextureFlags::ShaderResource;

				RHITextureDesc desc = {
				        .type   = RHITextureType::Texture2D,
				        .format = RHIColorFormat::R8G8B8A8,
				        .size   = {tex->Width, tex->Height, 1},
				        .mips   = 1,
				        .flags  = RHITextureFlags::ShaderResource,
				};

				RHITexture* texture = rhi->create_texture(desc);

				RHITextureRegion region = RHITextureRegion({tex->Width, tex->Height, 1});
				ctx->barrier(texture, RHIAccess::TransferDst);
				ctx->update(texture, region, tex->GetPixels(), {.size = static_cast<usize>(tex->GetSizeInBytes())});

				// Store identifiers
				tex->SetTexID(ImTextureID(texture));
				tex->SetStatus(ImTextureStatus_OK);
				tex->BackendUserData = texture;
			}
			else if (tex->Status == ImTextureStatus_WantUpdates)
			{
				RHITexture* texture = static_cast<RHITexture*>(tex->BackendUserData);
				ctx->barrier(texture, RHIAccess::TransferDst);

				for (ImTextureRect& r : tex->Updates)
				{
					RHITextureRegion region;
					region.offset = {r.x, r.y, 0};
					region.extent = {r.w, r.h, 1};

					u8* begin               = static_cast<u8*>(tex->GetPixelsAt(r.x, r.y));
					u8* end                 = static_cast<u8*>(tex->GetPixelsAt(r.x + r.w, r.y + r.h));
					const usize buffer_size = end - begin;

					ctx->update(texture, region, begin, {.size = buffer_size, .width = static_cast<u16>(tex->Width)});
				}

				tex->SetStatus(ImTextureStatus_OK);
			}
			else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
			{
				destroy_texture(tex);
			}
		}

		static void render(RHIContext* ctx, Window* window, ImDrawData* draw_data)
		{
			trinex_profile_cpu_n("ImGui");
			trinex_rhi_push_stage(ctx, "ImGui Render");

			// Avoid rendering when minimized
			if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			{
				trinex_rhi_pop_stage(ctx);
				return;
			}

			ImGuiTrinexData* bd         = backend_data();
			ImGuiTrinexViewportData* vd = reinterpret_cast<ImGuiTrinexViewportData*>(draw_data->OwnerViewport->RendererUserData);

			if (bd == nullptr || vd == nullptr)
			{
				trinex_rhi_pop_stage(ctx);
				return;
			}

			RHITexturePool* pool     = RHITexturePool::global_instance();
			const Vector2u view_size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y};

			bd->context = ctx;
			bd->target  = pool->acquire(RHISurfaceFormat::RGBA8, view_size, RHITextureFlags::ColorAttachment);

			trinex_rhi_push_stage(ctx, "ImGui Setup state");

			if (draw_data->Textures != nullptr)
			{
				for (ImTextureData* tex : *draw_data->Textures)
				{
					if (tex->Status != ImTextureStatus_OK)
					{
						update_texture(ctx, tex);
					}
				}
			}

			if (!vd->vertex_buffer || static_cast<int>(vd->vertex_count) < draw_data->TotalVtxCount)
			{
				constexpr auto flags = RHIBufferFlags::VertexBuffer;
				vd->vertex_count     = draw_data->TotalVtxCount + 5000;
				auto len             = vd->vertex_count * sizeof(ImDrawVert);
				vd->vertex_buffer    = RHI::instance()->create_buffer(len, flags);
			}

			if (!vd->index_buffer || static_cast<int>(vd->index_count) < draw_data->TotalIdxCount)
			{
				constexpr auto flags = RHIBufferFlags::IndexBuffer;
				vd->index_count      = draw_data->TotalIdxCount + 10000;
				auto len             = vd->index_count * sizeof(ImDrawIdx);
				vd->index_buffer     = RHI::instance()->create_buffer(len, flags);
			}

			// Upload vertex/index data into a single contiguous GPU buffer

			usize vtx_offset = 0;
			usize idx_offset = 0;

			{
				trinex_profile_cpu_n("Update Buffers");
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					usize vtx_size             = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
					usize idx_size             = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

					ctx->barrier(vd->vertex_buffer, RHIAccess::TransferDst);
					ctx->barrier(vd->index_buffer, RHIAccess::TransferDst);

					if (vtx_size > 0)
					{
						ctx->update(vd->vertex_buffer, cmd_list->VtxBuffer.Data, {.size = vtx_size, .dst_offset = vtx_offset});
						vtx_offset += vtx_size;
					}

					if (idx_size > 0)
					{
						ctx->update(vd->index_buffer, cmd_list->IdxBuffer.Data, {.size = idx_size, .dst_offset = idx_offset});
						idx_offset += idx_size;
					}
				}

				ctx->barrier(vd->vertex_buffer, RHIAccess::VertexBuffer);
				ctx->barrier(vd->index_buffer, RHIAccess::IndexBuffer);
			}

			// Apply barriers
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

					if (RHITexture* texture = pcmd->TexRef.GetTexID().texture)
					{
						ctx->barrier(texture, RHIAccess::SRVGraphics);
					}
				}
			}

			setup_render_state(draw_data);

			int global_idx_offset = 0;
			int global_vtx_offset = 0;
			ImVec2 clip_off       = draw_data->DisplayPos;

			ctx->bind_vertex_attribute(RHISemantic::Position, RHIVertexFormat::RG32F, 0, offsetof(ImDrawVert, pos));
			ctx->bind_vertex_attribute(RHISemantic::TexCoord0, RHIVertexFormat::RG32F, 0, offsetof(ImDrawVert, uv));
			ctx->bind_vertex_attribute(RHISemantic::Color, RHIVertexFormat::RGBA8, 0, offsetof(ImDrawVert, col));

			ctx->bind_vertex_buffer(vd->vertex_buffer, 0, sizeof(ImDrawVert), 0);
			ctx->bind_index_buffer(vd->index_buffer, RHIIndexFormat::UInt16);

			trinex_rhi_pop_stage(ctx);
			{
				trinex_profile_cpu_n("Render");

				ctx->barrier(bd->target, RHIAccess::TransferDst);
				ctx->clear_rtv(bd->target->as_rtv(), 0.f, 0.f, 0.f, 1.f);
				ctx->barrier(bd->target, RHIAccess::RTV);
				ctx->begin_rendering(bd->target->as_rtv());

				auto pipeline = ImGuiPipeline::instance();
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					trinex_rhi_push_stage(ctx, "ImGui Command list");
					const ImDrawList* cmd_list = draw_data->CmdLists[n];

					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

						if (pcmd->UserCallback != nullptr)
						{
							if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
							{
								ctx->end_rendering();
								setup_render_state(draw_data);
								ctx->begin_rendering(bd->target->as_rtv());
							}
							else
								pcmd->UserCallback(cmd_list, pcmd);
						}
						else
						{
							ImVec2 clip_min(Math::max(pcmd->ClipRect.x - clip_off.x, 0.f),
							                Math::max(pcmd->ClipRect.y - clip_off.y, 0.f));

							ImVec2 clip_max(Math::max(pcmd->ClipRect.z - clip_off.x, clip_min.x),
							                Math::max(pcmd->ClipRect.w - clip_off.y, clip_min.y));

							if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								continue;

							RHIScissor scissor;
							scissor.pos.x  = clip_min.x / draw_data->DisplaySize.x;
							scissor.pos.y  = clip_min.y / draw_data->DisplaySize.y;
							scissor.size.x = (clip_max.x - clip_min.x) / draw_data->DisplaySize.x;
							scissor.size.y = (clip_max.y - clip_min.y) / draw_data->DisplaySize.y;

							ctx->scissor(scissor);

							ImTextureID texture = pcmd->GetTexID();

							pipeline->bind(ctx, texture.texture ? texture.texture : bd->font_texture->rhi_texture());
							pipeline->bind(ctx, texture.sampler ? texture.sampler : RHIBilinearSampler::static_sampler());

							ctx->draw_indexed(RHITopology::TriangleList, pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset,
							                  pcmd->VtxOffset + global_vtx_offset);
						}
					}

					global_idx_offset += cmd_list->IdxBuffer.Size;
					global_vtx_offset += cmd_list->VtxBuffer.Size;
					trinex_rhi_pop_stage(ctx);
				}

				ctx->end_rendering();

				RHITexture* swapchain = window->render_viewport()->swapchain()->as_texture();

				ctx->barrier(bd->target, RHIAccess::TransferSrc);
				ctx->barrier(swapchain, RHIAccess::TransferDst);
				ctx->copy(swapchain, RHITextureRegion(swapchain->size()), bd->target, RHITextureRegion(view_size));
			}
			trinex_rhi_pop_stage(ctx);
			pool->release(bd->target);

			bd->context = nullptr;
			bd->target  = nullptr;
		}

		static void destroy_device_objects()
		{
			ImGuiTrinexData* bd         = backend_data();
			bool call_garbage_collector = !Trinex::engine_instance->is_shuting_down();

			for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
			{
				if (tex->RefCount == 1)
				{
					destroy_texture(tex);
				}
			}
		}

		static void create_device_objects()
		{
			ImGuiTrinexData* bd = backend_data();
			if (bd->font_texture)
				destroy_device_objects();

			ImGuiPipeline::create();
		}

		static void create_window(ImGuiViewport* viewport)
		{
			ImGuiTrinexViewportData* vd = IM_NEW(ImGuiTrinexViewportData)();
			viewport->RendererUserData  = vd;
		}

		static void destroy_window(ImGuiViewport* viewport)
		{
			if (ImGuiTrinexViewportData* vd = (ImGuiTrinexViewportData*) viewport->RendererUserData)
			{
				IM_DELETE(vd);
			}
			viewport->RendererUserData = nullptr;
		}

		static void init_platform_interface()
		{
			ImGuiPlatformIO& platform_io       = ImGui::GetPlatformIO();
			platform_io.Renderer_CreateWindow  = create_window;
			platform_io.Renderer_DestroyWindow = destroy_window;
		}

		static void shutdown_platform_interface()
		{
			// Do not destroy from render thread!
		}

		static bool init(Trinex::Window* window, ImGuiContext* ctx)
		{
			ImGui::SetCurrentContext(ctx);
			ImGuiIO& io = ImGui::GetIO();
			IMGUI_CHECKVERSION();
			IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

			ImGuiTrinexData* bd = IM_NEW(ImGuiTrinexData)();

			io.BackendRendererUserData = (void*) bd;
			io.BackendRendererName     = "imgui_impl_trinex";
			io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				init_platform_interface();

			create_device_objects();

			ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
			main_viewport->RendererUserData = IM_NEW(ImGuiTrinexViewportData)();
			return true;
		}

		static void shutdown(ImGuiContext* ctx)
		{
			ImGui::SetCurrentContext(ctx);
			ImGuiTrinexData* bd = backend_data();
			IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
			ImGuiIO& io = ImGui::GetIO();

			ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			if (auto vd = reinterpret_cast<ImGuiTrinexViewportData*>(main_viewport->RendererUserData))
				IM_DELETE(vd);
			main_viewport->RendererUserData = nullptr;

			shutdown_platform_interface();
			destroy_device_objects();
			io.BackendRendererName     = nullptr;
			io.BackendRendererUserData = nullptr;
			io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasViewports);
			IM_DELETE(bd);
		}
	}// namespace RenderBackend

	namespace WindowBackend
	{
		static Map<Trinex::Window*, FlatSet<ImGuiContext*>> s_window_mapping;

		struct ImGuiTrinexWindowData {
			Window* window;
			float time;
			bool update_monitors;

			ImGuiTrinexWindowData() : window(nullptr), time(engine_instance->time_seconds()), update_monitors(true) {}
		};

		class ImGuiViewportClient : public ViewportClient
		{
			trinex_class(ImGuiViewportClient, ViewportClient);
			ImGuiViewport* m_viewport = nullptr;
			ImGuiContext* m_context   = nullptr;

		public:
			inline ImGuiViewportClient& init(ImGuiViewport* viewport)
			{
				m_context = ImGui::GetCurrentContext();
				return *this;
			}

			inline ImGuiContext* context() const { return m_context; }

			ViewportClient& attach(class RenderViewport* viewport) override
			{
				// m_window = ImGuiWindow::current();
				return *this;
			}

			ViewportClient& update(class RenderViewport* viewport, float dt) override
			{
				// ImGuiContextLock lock(m_window->context());

				// RHISwapchain* swapchain = viewport->swapchain();

				// auto bd = RenderBackend::backend_data();
				// std::swap(viewport, bd->window);// Temporary set as main viewport

				// RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
				// {
				// 	RHITexture* texture = swapchain->as_texture();
				// 	ctx->barrier(texture, RHIAccess::RTV);

				// 	trinex_rhi_push_stage(ctx, "ImGuiViewportClient");
				// 	UI::Backend::imgui_render(ctx, m_viewport->DrawData);
				// 	trinex_rhi_pop_stage(ctx);

				// 	ctx->barrier(texture, RHIAccess::PresentSrc);
				// }
				// RHIContextPool::global_instance()->end_context(ctx, swapchain->acquire_semaphore(),
				//                                                swapchain->present_semaphore());
				// RHI::instance()->present(swapchain);

				// std::swap(viewport, bd->window);// Restore main viewport
				return *this;
			}
		};

		trinex_implement_class(Trinex::UI::Backend::WindowBackend::ImGuiViewportClient, 0) {}

		static ImGuiTrinexWindowData* backend_data()
		{
			return ImGui::GetCurrentContext() ? reinterpret_cast<ImGuiTrinexWindowData*>(ImGui::GetIO().BackendPlatformUserData)
			                                  : nullptr;
		}

		static FORCE_INLINE Trinex::Window* window_from(Identifier window_id)
		{
			return WindowManager::instance()->find(window_id);
		}

		struct ImGuiContextSaver {
			ImGuiContext* ctx;

			ImGuiContextSaver(ImGuiContext* new_context) : ctx(ImGui::GetCurrentContext())
			{
				ImGui::SetCurrentContext(new_context);
			}

			~ImGuiContextSaver() { ImGui::SetCurrentContext(ctx); }
		};

		static ImGuiMouseButton imgui_mouse_button_of(MouseButton button)
		{
			switch (button)
			{
				case MouseButton::Left: return ImGuiMouseButton_Left;
				case MouseButton::Middle: return ImGuiMouseButton_Middle;
				case MouseButton::Right: return ImGuiMouseButton_Right;
				case MouseButton::X1:
				case MouseButton::X2:
				default: return -1;
			}
		}

		static ImGuiKey imgui_key_of(ScanCode button)
		{
			switch (button)
			{
				case ScanCode::Space: return ImGuiKey_Space;
				case ScanCode::Apostrophe: return ImGuiKey_Apostrophe;
				case ScanCode::Comma: return ImGuiKey_Comma;
				case ScanCode::Minus: return ImGuiKey_Minus;
				case ScanCode::Period: return ImGuiKey_Period;
				case ScanCode::Slash: return ImGuiKey_Slash;
				case ScanCode::Num0: return ImGuiKey_0;
				case ScanCode::Num1: return ImGuiKey_1;
				case ScanCode::Num2: return ImGuiKey_2;
				case ScanCode::Num3: return ImGuiKey_3;
				case ScanCode::Num4: return ImGuiKey_4;
				case ScanCode::Num5: return ImGuiKey_5;
				case ScanCode::Num6: return ImGuiKey_6;
				case ScanCode::Num7: return ImGuiKey_7;
				case ScanCode::Num8: return ImGuiKey_8;
				case ScanCode::Num9: return ImGuiKey_9;
				case ScanCode::Semicolon: return ImGuiKey_Semicolon;
				case ScanCode::Equals: return ImGuiKey_Equal;
				case ScanCode::A: return ImGuiKey_A;
				case ScanCode::B: return ImGuiKey_B;
				case ScanCode::C: return ImGuiKey_C;
				case ScanCode::D: return ImGuiKey_D;
				case ScanCode::E: return ImGuiKey_E;
				case ScanCode::F: return ImGuiKey_F;
				case ScanCode::G: return ImGuiKey_G;
				case ScanCode::H: return ImGuiKey_H;
				case ScanCode::I: return ImGuiKey_I;
				case ScanCode::J: return ImGuiKey_J;
				case ScanCode::K: return ImGuiKey_K;
				case ScanCode::L: return ImGuiKey_L;
				case ScanCode::M: return ImGuiKey_M;
				case ScanCode::N: return ImGuiKey_N;
				case ScanCode::O: return ImGuiKey_O;
				case ScanCode::P: return ImGuiKey_P;
				case ScanCode::Q: return ImGuiKey_Q;
				case ScanCode::R: return ImGuiKey_R;
				case ScanCode::S: return ImGuiKey_S;
				case ScanCode::T: return ImGuiKey_T;
				case ScanCode::U: return ImGuiKey_U;
				case ScanCode::V: return ImGuiKey_V;
				case ScanCode::W: return ImGuiKey_W;
				case ScanCode::X: return ImGuiKey_X;
				case ScanCode::Y: return ImGuiKey_Y;
				case ScanCode::Z: return ImGuiKey_Z;
				case ScanCode::LeftBracket: return ImGuiKey_LeftBracket;
				case ScanCode::Backslash: return ImGuiKey_Backslash;
				case ScanCode::RightBracket: return ImGuiKey_RightBracket;
				case ScanCode::Grave: return ImGuiKey_GraveAccent;
				case ScanCode::Www: return ImGuiKey_None;
				case ScanCode::Escape: return ImGuiKey_Escape;
				case ScanCode::Return: return ImGuiKey_Enter;
				case ScanCode::Tab: return ImGuiKey_Tab;
				case ScanCode::Backspace: return ImGuiKey_Backspace;
				case ScanCode::Insert: return ImGuiKey_Insert;
				case ScanCode::Delete: return ImGuiKey_Delete;
				case ScanCode::Right: return ImGuiKey_RightArrow;
				case ScanCode::Left: return ImGuiKey_LeftArrow;
				case ScanCode::Down: return ImGuiKey_DownArrow;
				case ScanCode::Up: return ImGuiKey_UpArrow;
				case ScanCode::PageUp: return ImGuiKey_PageUp;
				case ScanCode::PageDown: return ImGuiKey_PageDown;
				case ScanCode::Home: return ImGuiKey_Home;
				case ScanCode::End: return ImGuiKey_End;
				case ScanCode::CapsLock: return ImGuiKey_CapsLock;
				case ScanCode::ScrollLock: return ImGuiKey_ScrollLock;
				case ScanCode::NumLockClear: return ImGuiKey_NumLock;
				case ScanCode::PrintScreen: return ImGuiKey_PrintScreen;
				case ScanCode::Pause: return ImGuiKey_Pause;
				case ScanCode::F1: return ImGuiKey_F1;
				case ScanCode::F2: return ImGuiKey_F2;
				case ScanCode::F3: return ImGuiKey_F3;
				case ScanCode::F4: return ImGuiKey_F4;
				case ScanCode::F5: return ImGuiKey_F5;
				case ScanCode::F6: return ImGuiKey_F6;
				case ScanCode::F7: return ImGuiKey_F7;
				case ScanCode::F8: return ImGuiKey_F8;
				case ScanCode::F9: return ImGuiKey_F9;
				case ScanCode::F10: return ImGuiKey_F10;
				case ScanCode::F11: return ImGuiKey_F11;
				case ScanCode::F12: return ImGuiKey_F12;
				case ScanCode::F13: return ImGuiKey_F13;
				case ScanCode::F14: return ImGuiKey_F14;
				case ScanCode::F15: return ImGuiKey_F15;
				case ScanCode::F16: return ImGuiKey_F16;
				case ScanCode::F17: return ImGuiKey_F17;
				case ScanCode::F18: return ImGuiKey_F18;
				case ScanCode::F19: return ImGuiKey_F19;
				case ScanCode::F20: return ImGuiKey_F20;
				case ScanCode::F21: return ImGuiKey_F21;
				case ScanCode::F22: return ImGuiKey_F22;
				case ScanCode::F23: return ImGuiKey_F23;
				case ScanCode::F24: return ImGuiKey_F24;
				case ScanCode::Kp0: return ImGuiKey_Keypad0;
				case ScanCode::Kp1: return ImGuiKey_Keypad1;
				case ScanCode::Kp2: return ImGuiKey_Keypad2;
				case ScanCode::Kp3: return ImGuiKey_Keypad3;
				case ScanCode::Kp4: return ImGuiKey_Keypad4;
				case ScanCode::Kp5: return ImGuiKey_Keypad5;
				case ScanCode::Kp6: return ImGuiKey_Keypad6;
				case ScanCode::Kp7: return ImGuiKey_Keypad7;
				case ScanCode::Kp8: return ImGuiKey_Keypad8;
				case ScanCode::Kp9: return ImGuiKey_Keypad9;
				case ScanCode::KpPeriod: return ImGuiKey_KeypadDecimal;
				case ScanCode::KpDivide: return ImGuiKey_KeypadDivide;
				case ScanCode::KpMultiply: return ImGuiKey_KeypadMultiply;
				case ScanCode::KpMinus: return ImGuiKey_KeypadSubtract;
				case ScanCode::KpPlus: return ImGuiKey_KeypadAdd;
				case ScanCode::KpEnter: return ImGuiKey_KeypadEnter;
				case ScanCode::KpEquals: return ImGuiKey_KeypadEqual;
				case ScanCode::LeftShift: return ImGuiKey_LeftShift;
				case ScanCode::LeftControl: return ImGuiKey_LeftCtrl;
				case ScanCode::LeftAlt: return ImGuiKey_LeftAlt;
				case ScanCode::LeftGui: return ImGuiKey_LeftSuper;
				case ScanCode::RightShift: return ImGuiKey_RightShift;
				case ScanCode::RightControl: return ImGuiKey_RightCtrl;
				case ScanCode::RightAlt: return ImGuiKey_RightAlt;
				case ScanCode::RightGui: return ImGuiKey_RightSuper;
				case ScanCode::Menu: return ImGuiKey_Menu;
				default: return ImGuiKey_None;
			}
		}

		template<typename F>
		static void for_each_context(Identifier window_id, F&& f)
		{
			Trinex::Window* window = window_from(window_id);

			if (window)
			{
				auto& list = s_window_mapping[window];

				for (ImGuiContext* ctx : list)
				{
					ImGuiContextSaver saver(ctx);
					f(window);
				}
			}
		}

		static void imgui_sent_mouse_position(Trinex::Window* window, float x, float y)
		{
			auto& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				auto info = Platform::monitor_info(window->monitor_index());
				auto pos  = window->position();
				io.AddMousePosEvent(x + pos.x, info.size.y - (y + pos.y));
			}
			else
			{
				io.AddMousePosEvent(x, window->size().y - y);
			}
		}

		static void on_mouse_move(Identifier window_id, const PointerEvent& data)
		{
			for_each_context(window_id, [&data](Trinex::Window* window) {
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				imgui_sent_mouse_position(window, data.screen_position.x, data.screen_position.y);
			});
		}

		static void on_mouse_button(Identifier window_id, MouseButton button, bool is_pressed)
		{
			auto imgui_button = imgui_mouse_button_of(button);

			if (imgui_button != -1)
			{
				for_each_context(window_id, [is_pressed, imgui_button](Trinex::Window*) {
					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
					io.AddMouseButtonEvent(imgui_button, is_pressed);
				});
			}
		}

		static void on_mouse_wheel(Identifier window_id, const PointerEvent& data)
		{
			for_each_context(window_id, [&data](Trinex::Window*) {
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseWheelEvent(data.wheel_delta.x, data.wheel_delta.y);
			});
		}

		static void on_keyboard_button(Identifier window_id, ScanCode button, bool is_pressed)
		{
			auto imgui_button = imgui_key_of(button);

			if (imgui_button != ImGuiKey_None)
			{
				for_each_context(window_id, [is_pressed, imgui_button](Trinex::Window*) {
					auto& io = ImGui::GetIO();

					if (is_in<ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl>(imgui_button))
						io.AddKeyEvent(ImGuiMod_Ctrl, is_pressed);
					if (is_in<ImGuiKey_LeftShift, ImGuiKey_RightShift>(imgui_button))
						io.AddKeyEvent(ImGuiMod_Shift, is_pressed);
					if (is_in<ImGuiKey_LeftAlt, ImGuiKey_RightAlt>(imgui_button))
						io.AddKeyEvent(ImGuiMod_Alt, is_pressed);
					if (is_in<ImGuiKey_LeftSuper, ImGuiKey_RightSuper>(imgui_button))
						io.AddKeyEvent(ImGuiMod_Super, is_pressed);
					io.AddKeyEvent(imgui_button, is_pressed);
				});
			}
		}

		static void on_text_input(Identifier window_id, const TextInputEvent& data)
		{
			for_each_context(window_id, [&data](Trinex::Window*) {
				auto& io = ImGui::GetIO();
				io.AddInputCharacter(static_cast<unsigned int>(data.codepoint));
			});
		}

		static void on_window_close(Identifier window_id)
		{
			for_each_context(window_id, [](Trinex::Window* window) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestClose = true;
				}
			});
		}

		static void on_window_move(Identifier window_id)
		{
			for_each_context(window_id, [](Trinex::Window* window) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestMove = true;
				}
			});
		}

		static void on_window_resize(Identifier window_id)
		{
			for_each_context(window_id, [](Trinex::Window* window) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestResize = true;
				}
			});
		}

		class ImGuiEventListener final : public EventListener
		{
		public:
			EventDispatchResult on_event(RoutedEvent& event) override
			{
				switch (event.header.type_id)
				{
					case EventTypeIds::Pointer:
					{
						auto* data = reinterpret_cast<const PointerEvent*>(event.payload);
						if (data == nullptr)
							return {};

						switch (data->kind)
						{
							case PointerEventKind::Moved: on_mouse_move(event.header.window_id, *data); break;
							case PointerEventKind::ButtonPressed:
								on_mouse_button(event.header.window_id, static_cast<MouseButton::Enum>(data->button), true);
								break;
							case PointerEventKind::ButtonReleased:
								on_mouse_button(event.header.window_id, static_cast<MouseButton::Enum>(data->button), false);
								break;
							case PointerEventKind::Wheel: on_mouse_wheel(event.header.window_id, *data); break;
							default: break;
						}
						break;
					}

					case EventTypeIds::Key:
					{
						auto* data = reinterpret_cast<const KeyEvent*>(event.payload);
						if (data)
						{
							on_keyboard_button(event.header.window_id, static_cast<ScanCode::Enum>(data->scan_code),
							                   data->kind != KeyEventKind::Released);
						}
						break;
					}

					case EventTypeIds::TextInput:
					{
						auto* data = reinterpret_cast<const TextInputEvent*>(event.payload);
						if (data)
						{
							on_text_input(event.header.window_id, *data);
						}
						break;
					}

					case EventTypeIds::Window:
					{
						auto* data = reinterpret_cast<const WindowEvent*>(event.payload);
						if (data == nullptr)
							return {};

						switch (data->kind)
						{
							case WindowEventKind::CloseRequested: on_window_close(event.header.window_id); break;
							case WindowEventKind::Moved: on_window_move(event.header.window_id); break;
							case WindowEventKind::Resized: on_window_resize(event.header.window_id); break;
							default: break;
						}
						break;
					}

					default: break;
				}

				return {};
			}
		};

		static ImGuiEventListener m_listener;
		static bool m_listener_enabled = false;

		void disable_events()
		{
			if (!m_listener_enabled)
				return;

			if (EventSystem* system = EventSystem::instance())
			{
				system->dispatcher().remove_listener(EventTypeIds::Pointer, &m_listener);
				system->dispatcher().remove_listener(EventTypeIds::Key, &m_listener);
				system->dispatcher().remove_listener(EventTypeIds::TextInput, &m_listener);
				system->dispatcher().remove_listener(EventTypeIds::Window, &m_listener);
			}

			m_listener_enabled = false;
		}

		void enable_events()
		{
			if (m_listener_enabled)
				return;

			if (EventSystem* system = EventSystem::instance())
			{
				system->dispatcher().add_listener(EventTypeIds::Pointer, &m_listener);
				system->dispatcher().add_listener(EventTypeIds::Key, &m_listener);
				system->dispatcher().add_listener(EventTypeIds::TextInput, &m_listener);
				system->dispatcher().add_listener(EventTypeIds::Window, &m_listener);
				m_listener_enabled = true;
			}
		}

		static FORCE_INLINE Trinex::Window* window_from(ImGuiViewport* vp)
		{
			return reinterpret_cast<Trinex::Window*>(vp->PlatformHandle);
		}

		static void window_create(ImGuiViewport* vp)
		{
			WindowConfig config;
			config.position.x = vp->Pos.x;
			config.position.y = vp->Pos.y;
			config.size.x     = vp->Size.x;
			config.size.y     = vp->Size.y;

			config.attributes = {WindowAttribute::Hidden};

			if (vp->Flags & ImGuiViewportFlags_NoDecoration)
			{
				config.attributes.insert(WindowAttribute::BorderLess);
			}
			else
			{
				config.attributes.insert(WindowAttribute::Resizable);
			}

			config.client = "";

			auto parent_window = window_from(ImGui::FindViewportByID(vp->ParentViewportId));
			auto new_window    = Trinex::WindowManager::instance()->create_window(config, parent_window);

			auto render_viewport = new_window->render_viewport();
			auto client          = Object::new_instance<ImGuiViewportClient>();
			client->init(vp);
			render_viewport->client(client);

			vp->PlatformHandle   = new_window;
			vp->PlatformUserData = reinterpret_cast<void*>(new_window->id());

			new_window->on_destroy.push([vp](Window* window) {
				vp->PlatformHandle   = nullptr;
				vp->PlatformUserData = nullptr;
			});
		}

		static void window_destroy(ImGuiViewport* vp)
		{
			if (vp->ParentViewportId != 0)
			{
				Identifier id = reinterpret_cast<Identifier>(vp->PlatformUserData);
				if (Trinex::Window* window = WindowManager::instance()->find(id))
				{
					WindowManager::instance()->destroy_window(window);
				}
			}

			vp->PlatformUserData = vp->PlatformHandle = nullptr;
		}

		static void window_show(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				wd->show();
			}
		}

		static void set_window_pos(ImGuiViewport* vp, ImVec2 pos)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				auto info = Platform::monitor_info(wd->monitor_index());
				wd->position({pos.x, info.size.y - (pos.y + wd->size().y)});
			}
		}

		static ImVec2 get_window_pos(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				auto pos    = wd->position();
				auto info   = Platform::monitor_info(wd->monitor_index());
				float new_y = -pos.y + info.size.y - wd->size().y;
				return {static_cast<float>(pos.x), static_cast<float>(new_y)};
			}

			return {0, 0};
		}

		static void set_window_size(ImGuiViewport* vp, ImVec2 size)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				wd->size({size.x, size.y});
			}
		}

		static ImVec2 get_window_size(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				auto size = wd->size();
				return {static_cast<float>(size.x), static_cast<float>(size.y)};
			}

			return {0, 0};
		}

		static bool get_window_focus(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				return wd->focused();
			}
			return false;
		}

		static void set_window_focus(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				wd->focus();
			}
		}

		static void set_window_title(ImGuiViewport* vp, const char* title)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				wd->title(title);
			}
		}

		static bool get_window_minimized(ImGuiViewport* vp)
		{
			if (Trinex::Window* wd = window_from(vp))
			{
				return wd->is_iconify();
			}
			return false;
		}

		static void init_platform_interface(Trinex::Window* window)
		{
			ImGuiPlatformIO& platform_io            = ImGui::GetPlatformIO();
			platform_io.Platform_CreateWindow       = window_create;
			platform_io.Platform_DestroyWindow      = window_destroy;
			platform_io.Platform_ShowWindow         = window_show;
			platform_io.Platform_SetWindowPos       = set_window_pos;
			platform_io.Platform_GetWindowPos       = get_window_pos;
			platform_io.Platform_SetWindowSize      = set_window_size;
			platform_io.Platform_GetWindowSize      = get_window_size;
			platform_io.Platform_SetWindowFocus     = set_window_focus;
			platform_io.Platform_GetWindowFocus     = get_window_focus;
			platform_io.Platform_GetWindowMinimized = get_window_minimized;
			platform_io.Platform_SetWindowTitle     = set_window_title;

			ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
			main_viewport->PlatformHandle   = window;
			main_viewport->PlatformUserData = reinterpret_cast<void*>(window->id());
		}

		static void init(Trinex::Window* window, ImGuiContext* context)
		{
			s_window_mapping[window].insert(context);

			ImGuiIO& io            = ImGui::GetIO();
			io.BackendPlatformName = "imgui_impl_trinex";
			auto bd                = IM_NEW(ImGuiTrinexWindowData)();

			enable_events();
			io.BackendPlatformUserData = bd;
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;


			if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
				init_platform_interface(window);
		}

		static void shutdown(Trinex::Window* window)
		{
			ImGui::DestroyPlatformWindows();

			ImGuiIO& io            = ImGui::GetIO();
			io.BackendPlatformName = nullptr;
			IM_DELETE(reinterpret_cast<ImGuiTrinexWindowData*>(io.BackendPlatformUserData));
			io.BackendPlatformUserData = nullptr;

			s_window_mapping.erase(window);
		}

		static void update_monitors()
		{
			auto* bd                     = backend_data();
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Monitors.resize(0);
			bd->update_monitors = false;

			usize display_count = Platform::monitors_count();
			for (usize n = 0; n < display_count; n++)
			{
				ImGuiPlatformMonitor monitor;
				auto info       = Platform::monitor_info(n);
				monitor.WorkPos = monitor.MainPos = ImVec2(info.pos.x, info.pos.y);
				monitor.WorkSize = monitor.MainSize = ImVec2(info.size.x, info.size.y);
				monitor.DpiScale                    = info.dpi / 96.0f;

				monitor.PlatformHandle = reinterpret_cast<void*>(n);
				platform_io.Monitors.push_back(monitor);
			}
		}

		static void new_frame(Trinex::Window* window)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto bd     = backend_data();

			Vector2f size          = Vector2f(window->size());
			Vector2f drawable_size = Vector2f(window->size());

			io.DisplaySize = ImVec2(size.x, size.y);
			if (drawable_size.x > 0 && drawable_size.y > 0)
				io.DisplayFramebufferScale = ImVec2(drawable_size.x / size.x, drawable_size.y / size.y);

			// Update monitors
			if (bd->update_monitors)
				update_monitors();

			float current_time = engine_instance->time_seconds();
			io.DeltaTime       = bd->time > 0.0 ? (current_time - bd->time) : (1.0f / 60.0f);
			bd->time           = current_time;
		}
	}// namespace WindowBackend

	RHIContext* rhi()
	{
		return RenderBackend::backend_data()->context;
	}

	RHITexture* render_target()
	{
		return RenderBackend::backend_data()->target;
	}

	void imgui_init(Window* window, ImGuiContext* context)
	{
		WindowBackend::init(window, context);
		RenderBackend::init(window, context);
	}

	void imgui_shutdown(Window* window, ImGuiContext* context)
	{
		RenderBackend::shutdown(context);
		WindowBackend::shutdown(window);
	}

	void imgui_new_frame(Window* window)
	{
		WindowBackend::new_frame(window);
	}

	void imgui_render(RHIContext* ctx, Window* window, ImDrawData* data)
	{
		RenderBackend::render(ctx, window, data);
	}

	void imgui_event_recieved(const Event& event) {}

	void imgui_disable_events() {}

	void imgui_enable_events() {}
}// namespace Trinex::UI::Backend
