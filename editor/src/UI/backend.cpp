#include <UI/backend.hpp>

#include <Core/base_engine.hpp>
#include <Core/etl/flat_set.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/templates.hpp>
#include <Core/math/math.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Platform/platform.hpp>
#include <RHI/context.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <Systems/event_system.hpp>
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

			RHITexturePool* pool = RHITexturePool::global_instance();

			const Vector2u view_size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y};

			bd->context = ctx;
			bd->target  = pool->request_surface(RHISurfaceFormat::RGBA8, view_size, RHITextureFlags::ColorAttachment);

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
				ctx->copy(swapchain, bd->target, RHITextureRegion(view_size));
			}
			trinex_rhi_pop_stage(ctx);
			pool->return_surface(bd->target);

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

			ViewportClient& on_bind_viewport(class RenderViewport* viewport) override
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

		static FORCE_INLINE Trinex::Window* window_from(const Event& event)
		{
			return WindowManager::instance()->find(event.window_id);
		}

		struct ImGuiContextSaver {
			ImGuiContext* ctx;

			ImGuiContextSaver(ImGuiContext* new_context) : ctx(ImGui::GetCurrentContext())
			{
				ImGui::SetCurrentContext(new_context);
			}

			~ImGuiContextSaver() { ImGui::SetCurrentContext(ctx); }
		};

		static ImGuiMouseButton imgui_button_of(Mouse::Button button)
		{
			switch (button)
			{
				case Mouse::Button::Left: return ImGuiMouseButton_Left;
				case Mouse::Button::Middle: return ImGuiMouseButton_Middle;
				case Mouse::Button::Right: return ImGuiMouseButton_Right;
				case Mouse::Button::Forward:
				case Mouse::Button::Back:
				default: return -1;
			}
		}

		static ImGuiKey imgui_button_of(Keyboard::Key button)
		{
			switch (button)
			{
				case Keyboard::Key::Space: return ImGuiKey_Space;
				case Keyboard::Key::Apostrophe: return ImGuiKey_Apostrophe;
				case Keyboard::Key::Comma: return ImGuiKey_Comma;
				case Keyboard::Key::Minus: return ImGuiKey_Minus;
				case Keyboard::Key::Period: return ImGuiKey_Period;
				case Keyboard::Key::Slash: return ImGuiKey_Slash;
				case Keyboard::Key::Num0: return ImGuiKey_0;
				case Keyboard::Key::Num1: return ImGuiKey_1;
				case Keyboard::Key::Num2: return ImGuiKey_2;
				case Keyboard::Key::Num3: return ImGuiKey_3;
				case Keyboard::Key::Num4: return ImGuiKey_4;
				case Keyboard::Key::Num5: return ImGuiKey_5;
				case Keyboard::Key::Num6: return ImGuiKey_6;
				case Keyboard::Key::Num7: return ImGuiKey_7;
				case Keyboard::Key::Num8: return ImGuiKey_8;
				case Keyboard::Key::Num9: return ImGuiKey_9;
				case Keyboard::Key::Semicolon: return ImGuiKey_Semicolon;
				case Keyboard::Key::Equal: return ImGuiKey_Equal;
				case Keyboard::Key::A: return ImGuiKey_A;
				case Keyboard::Key::B: return ImGuiKey_B;
				case Keyboard::Key::C: return ImGuiKey_C;
				case Keyboard::Key::D: return ImGuiKey_D;
				case Keyboard::Key::E: return ImGuiKey_E;
				case Keyboard::Key::F: return ImGuiKey_F;
				case Keyboard::Key::G: return ImGuiKey_G;
				case Keyboard::Key::H: return ImGuiKey_H;
				case Keyboard::Key::I: return ImGuiKey_I;
				case Keyboard::Key::J: return ImGuiKey_J;
				case Keyboard::Key::K: return ImGuiKey_K;
				case Keyboard::Key::L: return ImGuiKey_L;
				case Keyboard::Key::M: return ImGuiKey_M;
				case Keyboard::Key::N: return ImGuiKey_N;
				case Keyboard::Key::O: return ImGuiKey_O;
				case Keyboard::Key::P: return ImGuiKey_P;
				case Keyboard::Key::Q: return ImGuiKey_Q;
				case Keyboard::Key::R: return ImGuiKey_R;
				case Keyboard::Key::S: return ImGuiKey_S;
				case Keyboard::Key::T: return ImGuiKey_T;
				case Keyboard::Key::U: return ImGuiKey_U;
				case Keyboard::Key::V: return ImGuiKey_V;
				case Keyboard::Key::W: return ImGuiKey_W;
				case Keyboard::Key::X: return ImGuiKey_X;
				case Keyboard::Key::Y: return ImGuiKey_Y;
				case Keyboard::Key::Z: return ImGuiKey_Z;
				case Keyboard::Key::LeftBracket: return ImGuiKey_LeftBracket;
				case Keyboard::Key::Backslash: return ImGuiKey_Backslash;
				case Keyboard::Key::RightBracket: return ImGuiKey_RightBracket;
				case Keyboard::Key::GraveAccent: return ImGuiKey_GraveAccent;
				case Keyboard::Key::Explorer: return ImGuiKey_None;
				case Keyboard::Key::Escape: return ImGuiKey_Escape;
				case Keyboard::Key::Enter: return ImGuiKey_Enter;
				case Keyboard::Key::Tab: return ImGuiKey_Tab;
				case Keyboard::Key::Backspace: return ImGuiKey_Backspace;
				case Keyboard::Key::Insert: return ImGuiKey_Insert;
				case Keyboard::Key::Delete: return ImGuiKey_Delete;
				case Keyboard::Key::Right: return ImGuiKey_RightArrow;
				case Keyboard::Key::Left: return ImGuiKey_LeftArrow;
				case Keyboard::Key::Down: return ImGuiKey_DownArrow;
				case Keyboard::Key::Up: return ImGuiKey_UpArrow;
				case Keyboard::Key::PageUp: return ImGuiKey_PageUp;
				case Keyboard::Key::PageDown: return ImGuiKey_PageDown;
				case Keyboard::Key::Home: return ImGuiKey_Home;
				case Keyboard::Key::End: return ImGuiKey_End;
				case Keyboard::Key::CapsLock: return ImGuiKey_CapsLock;
				case Keyboard::Key::ScrollLock: return ImGuiKey_ScrollLock;
				case Keyboard::Key::NumLock: return ImGuiKey_NumLock;
				case Keyboard::Key::PrintScreen: return ImGuiKey_PrintScreen;
				case Keyboard::Key::Pause: return ImGuiKey_Pause;
				case Keyboard::Key::F1: return ImGuiKey_F1;
				case Keyboard::Key::F2: return ImGuiKey_F2;
				case Keyboard::Key::F3: return ImGuiKey_F3;
				case Keyboard::Key::F4: return ImGuiKey_F4;
				case Keyboard::Key::F5: return ImGuiKey_F5;
				case Keyboard::Key::F6: return ImGuiKey_F6;
				case Keyboard::Key::F7: return ImGuiKey_F7;
				case Keyboard::Key::F8: return ImGuiKey_F8;
				case Keyboard::Key::F9: return ImGuiKey_F9;
				case Keyboard::Key::F10: return ImGuiKey_F10;
				case Keyboard::Key::F11: return ImGuiKey_F11;
				case Keyboard::Key::F12: return ImGuiKey_F12;
				case Keyboard::Key::F13: return ImGuiKey_F13;
				case Keyboard::Key::F14: return ImGuiKey_F14;
				case Keyboard::Key::F15: return ImGuiKey_F15;
				case Keyboard::Key::F16: return ImGuiKey_F16;
				case Keyboard::Key::F17: return ImGuiKey_F17;
				case Keyboard::Key::F18: return ImGuiKey_F18;
				case Keyboard::Key::F19: return ImGuiKey_F19;
				case Keyboard::Key::F20: return ImGuiKey_F20;
				case Keyboard::Key::F21: return ImGuiKey_F21;
				case Keyboard::Key::F22: return ImGuiKey_F22;
				case Keyboard::Key::F23: return ImGuiKey_F23;
				case Keyboard::Key::F24: return ImGuiKey_F24;
				case Keyboard::Key::Kp0: return ImGuiKey_Keypad0;
				case Keyboard::Key::Kp1: return ImGuiKey_Keypad1;
				case Keyboard::Key::Kp2: return ImGuiKey_Keypad2;
				case Keyboard::Key::Kp3: return ImGuiKey_Keypad3;
				case Keyboard::Key::Kp4: return ImGuiKey_Keypad4;
				case Keyboard::Key::Kp5: return ImGuiKey_Keypad5;
				case Keyboard::Key::Kp6: return ImGuiKey_Keypad6;
				case Keyboard::Key::Kp7: return ImGuiKey_Keypad7;
				case Keyboard::Key::Kp8: return ImGuiKey_Keypad8;
				case Keyboard::Key::Kp9: return ImGuiKey_Keypad9;
				case Keyboard::Key::KpDot: return ImGuiKey_KeypadDecimal;
				case Keyboard::Key::KpDivide: return ImGuiKey_KeypadDivide;
				case Keyboard::Key::KpMultiply: return ImGuiKey_KeypadMultiply;
				case Keyboard::Key::KpSubtract: return ImGuiKey_KeypadSubtract;
				case Keyboard::Key::KpAdd: return ImGuiKey_KeypadAdd;
				case Keyboard::Key::KpEnter: return ImGuiKey_KeypadEnter;
				case Keyboard::Key::KpEqual: return ImGuiKey_KeypadEqual;
				case Keyboard::Key::LeftShift: return ImGuiKey_LeftShift;
				case Keyboard::Key::LeftControl: return ImGuiKey_LeftCtrl;
				case Keyboard::Key::LeftAlt: return ImGuiKey_LeftAlt;
				case Keyboard::Key::LeftSuper: return ImGuiKey_LeftSuper;
				case Keyboard::Key::RightShift: return ImGuiKey_RightShift;
				case Keyboard::Key::RightControl: return ImGuiKey_RightCtrl;
				case Keyboard::Key::RightAlt: return ImGuiKey_RightAlt;
				case Keyboard::Key::RightSuper: return ImGuiKey_RightSuper;
				case Keyboard::Key::Menu: return ImGuiKey_Menu;
				default: return ImGuiKey_None;
			}
		}

		template<typename F, typename... Args>
		static void for_each_context(const Event& event, F&& f)
		{
			ImGuiContext* context  = nullptr;
			Trinex::Window* window = window_from(event);

			if (window)
			{
				auto& list = s_window_mapping[window];

				for (ImGuiContext* ctx : list)
				{
					ImGuiContextSaver saver(ctx);
					f(window, event);
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

		static void on_mouse_move(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				auto& data = event.mouse.motion;
				auto& io   = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);

				imgui_sent_mouse_position(window, data.x, data.y);
			});
		}

		static void on_mouse_button(const Event& event, bool is_pressed)
		{
			auto& data        = event.mouse.button;
			auto imgui_button = imgui_button_of(data.button);

			if (imgui_button != -1)
			{
				for_each_context(event, [is_pressed, imgui_button](Trinex::Window* window, const Event& event) {
					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
					io.AddMouseButtonEvent(imgui_button, is_pressed);
				});
			}
		}

		static void on_mouse_wheel(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				auto& data = event.mouse.wheel;
				auto& io   = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseWheelEvent(data.x, data.y);
			});
		}

		static void on_keyboard_button(const Event& event, bool is_pressed)
		{
			auto& data        = event.keyboard;
			auto imgui_button = imgui_button_of(data.key);

			if (imgui_button != ImGuiKey_None)
			{
				for_each_context(event, [is_pressed, imgui_button](Trinex::Window* window, const Event& event) {
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

		static void on_text_input(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				auto& data = event.text_input;
				auto& io   = ImGui::GetIO();

				io.AddInputCharactersUTF8(data.text);
			});
		}

		static void on_window_close(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestClose = true;
				}
			});
		}

		static void on_window_move(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestMove = true;
				}
			});
		}

		static void on_window_resize(const Event& event)
		{
			for_each_context(event, [](Trinex::Window* window, const Event& event) {
				if (auto* vp = ImGui::FindViewportByPlatformHandle(window))
				{
					vp->PlatformRequestResize = true;
				}
			});
		}

		static void on_finger_down(const Event& event)
		{
			const auto& data = event.touchscreen.finger;

			if (data.index == 0)
			{
				for_each_context(event, [](Trinex::Window* window, const Event& event) {
					const auto& data = event.touchscreen.finger;

					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
					imgui_sent_mouse_position(window, data.x, data.y);
					io.AddMouseButtonEvent(0, true);
				});
			}
		}

		static void on_finger_up(const Event& event)
		{
			const auto& data = event.touchscreen.finger;

			if (data.index == 0)
			{
				for_each_context(event, [](Trinex::Window* window, const Event& event) {
					const auto& data = event.touchscreen.finger;

					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
					imgui_sent_mouse_position(window, data.x, data.y);
					io.AddMouseButtonEvent(0, false);
				});
			}
		}

		static void on_finger_motion(const Event& event)
		{
			const auto& data = event.touchscreen.finger_motion;

			if (data.index == 0)
			{
				for_each_context(event, [](Trinex::Window* window, const Event& event) {
					const auto& data = event.touchscreen.finger_motion;

					auto& io = ImGui::GetIO();
					io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
					imgui_sent_mouse_position(window, data.x, data.y);
				});
			}
		}

		void on_event_recieved(const Event& event)
		{
			auto type = event.type;

			switch (type)
			{
				case EventType::MouseMotion: return on_mouse_move(event);

				case EventType::MouseButtonDown:
				case EventType::MouseButtonUp: return on_mouse_button(event, type == EventType::MouseButtonDown);

				case EventType::MouseWheel: return on_mouse_wheel(event);

				case EventType::KeyDown:
				case EventType::KeyUp: return on_keyboard_button(event, type == EventType::KeyDown);

				case EventType::TextInput: return on_text_input(event);

				case EventType::WindowClose: return on_window_close(event);
				case EventType::WindowMoved: return on_window_move(event);
				case EventType::WindowResized: return on_window_resize(event);

				case EventType::FingerDown: return on_finger_down(event);
				case EventType::FingerUp: return on_finger_up(event);
				case EventType::FingerMotion: return on_finger_motion(event);

				default: break;
			}
		}

		static Identifier m_listener_id = 0;

		void disable_events()
		{
			if (m_listener_id == 0)
				return;

			EventSystem* system = EventSystem::instance();
			system->remove_listener(m_listener_id);
			m_listener_id = 0;
		}

		void enable_events()
		{
			if (m_listener_id)
				return;

			EventSystem* system = EventSystem::instance();
			m_listener_id       = system->add_listener(EventType::Undefined, on_event_recieved);
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
