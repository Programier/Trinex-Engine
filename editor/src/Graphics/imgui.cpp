#include <Core/base_engine.hpp>
#include <Core/editor_resources.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/etl/templates.hpp>
#include <Core/event.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/keyboard.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Core/mouse.hpp>
#include <Core/package.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/thread.hpp>
#include <Core/threading.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_cache.hpp>
#include <Graphics/texture.hpp>
#include <Platform/platform.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <Systems/event_system.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace Engine
{
	//////////////////////////// IMGUI RHI IMPLEMENTATION ////////////////////////////

	namespace ImGuiBackend_RHI
	{
		class ImGuiPipeline : public GlobalGraphicsPipeline
		{
			trinex_declare_pipeline(ImGuiPipeline, GlobalGraphicsPipeline);

		public:
			const RHIShaderParameterInfo* texture_parameter = nullptr;
			const RHIShaderParameterInfo* sampler_parameter = nullptr;
			const RHIShaderParameterInfo* model_parameter   = nullptr;
			Matrix4f model;
			RHIShaderResourceView* srv = nullptr;

			void push(RHIContext* ctx);
			void pop(RHIContext* ctx);
			void apply(RHIContext* ctx, const Sampler& sampler);
		};

		trinex_implement_pipeline(ImGuiPipeline, "[shaders_dir]:/TrinexEditor/imgui.slang")
		{
			texture_parameter = find_parameter("texture");
			sampler_parameter = find_parameter("sampler");
			model_parameter   = find_parameter("model");
		}

		void ImGuiPipeline::push(RHIContext* ctx)
		{
			ctx->push_pipeline(rhi_pipeline());
			ctx->push_depth_state(RHIDepthState(false, RHICompareFunc::Always, false));
			ctx->push_stencil_state(RHIStencilState());
			ctx->push_blending_state(RHIBlendingState(true));
		}

		void ImGuiPipeline::pop(RHIContext* ctx)
		{
			ctx->pop_pipeline();
			ctx->pop_depth_state();
			ctx->pop_stencil_state();
			ctx->pop_blending_state();
		}

		void ImGuiPipeline::apply(RHIContext* ctx, const Sampler& sampler)
		{
			RHIDescriptor sampler_descriptor = sampler.rhi_sampler()->descriptor();
			RHIDescriptor texture_descriptor = srv->descriptor();

			ctx->update_scalar(&sampler_descriptor, sampler_parameter);
			ctx->update_scalar(&texture_descriptor, texture_parameter);
			ctx->update_scalar(&model, sizeof(model), 0, model_parameter->binding);

			srv = nullptr;
		}


		bool imgui_trinex_rhi_init(Engine::Window*, ImGuiContext* ctx);
		void imgui_trinex_rhi_shutdown(ImGuiContext* ctx);
		void imgui_trinex_rhi_render_draw_data(RHIContext* ctx, ImDrawData* draw_data);

		struct ImGuiTrinexData {
			Texture2D* font_texture        = nullptr;
			Engine::RenderViewport* window = nullptr;
			Sampler sampler;
		};

		struct ImGuiTrinexViewportData {
			RHIResourcePtr<RHIBuffer> vertex_buffer;
			RHIResourcePtr<RHIBuffer> index_buffer;
			uint64_t vertex_count = 0;
			uint64_t index_count  = 0;
		};

		static ImGuiTrinexData* imgui_trinex_backend_data()
		{
			return ImGui::GetCurrentContext() ? (ImGuiTrinexData*) ImGui::GetIO().BackendRendererUserData : nullptr;
		}

		// RENDERING FUNCTIONS

		static void imgui_trinex_setup_render_state(RHIContext* ctx, ImDrawData* draw_data)
		{
			auto bd       = imgui_trinex_backend_data();
			auto pipeline = ImGuiPipeline::instance();
			pipeline->srv = nullptr;

			RHIViewport viewport(bd->window->size());
			ctx->barrier(bd->window->rhi_texture(), RHIAccess::RTV);
			ctx->bind_render_target1(bd->window->rhi_rtv());
			ctx->viewport(viewport);

			float L = draw_data->DisplayPos.x;
			float R = L + draw_data->DisplaySize.x;
			float T = draw_data->DisplayPos.y;
			float B = T + draw_data->DisplaySize.y;

			pipeline->model = Math::ortho(L, R, T, B, 0.f, 1.f);
		}

		void imgui_trinex_rhi_render_draw_data(RHIContext* ctx, ImDrawData* draw_data)
		{
			trinex_profile_cpu_n("ImGui");
			trinex_rhi_push_stage(ctx, "ImGui Render");

			// Avoid rendering when minimized
			if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
			{
				trinex_rhi_pop_stage(ctx);
				return;
			}

			ImGuiTrinexData* bd         = imgui_trinex_backend_data();
			ImGuiTrinexViewportData* vd = reinterpret_cast<ImGuiTrinexViewportData*>(draw_data->OwnerViewport->RendererUserData);

			if (bd == nullptr || vd == nullptr)
			{
				trinex_rhi_pop_stage(ctx);
				return;
			}

			trinex_rhi_push_stage(ctx, "ImGui Setup state");

			if (!vd->vertex_buffer || static_cast<int>(vd->vertex_count) < draw_data->TotalVtxCount)
			{
				vd->vertex_count = draw_data->TotalVtxCount + 5000;
				auto len         = vd->vertex_count * sizeof(ImDrawVert);
				vd->vertex_buffer =
				        rhi->create_buffer(len, nullptr, RHIBufferCreateFlags::Dynamic | RHIBufferCreateFlags::VertexBuffer);
			}

			if (!vd->index_buffer || static_cast<int>(vd->index_count) < draw_data->TotalIdxCount)
			{
				vd->index_count = draw_data->TotalIdxCount + 10000;
				auto len        = vd->index_count * sizeof(ImDrawIdx);
				vd->index_buffer =
				        rhi->create_buffer(len, nullptr, RHIBufferCreateFlags::Dynamic | RHIBufferCreateFlags::IndexBuffer);
			}

			// Upload vertex/index data into a single contiguous GPU buffer

			size_t vtx_offset = 0;
			size_t idx_offset = 0;

			{
				trinex_profile_cpu_n("Update Buffers");
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					const ImDrawList* cmd_list = draw_data->CmdLists[n];
					size_t vtx_size            = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
					size_t idx_size            = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

					ctx->barrier(vd->vertex_buffer, RHIAccess::TransferDst);
					ctx->barrier(vd->index_buffer, RHIAccess::TransferDst);

					if (vtx_size > 0)
					{
						ctx->update_buffer(vd->vertex_buffer, vtx_offset, vtx_size,
						                   reinterpret_cast<const byte*>(cmd_list->VtxBuffer.Data));
						vtx_offset += vtx_size;
					}

					if (idx_size > 0)
					{
						ctx->update_buffer(vd->index_buffer, idx_offset, idx_size,
						                   reinterpret_cast<const byte*>(cmd_list->IdxBuffer.Data));
						idx_offset += idx_size;
					}
				}

				ctx->barrier(vd->vertex_buffer, RHIAccess::VertexBuffer);
				ctx->barrier(vd->index_buffer, RHIAccess::IndexBuffer);
			}

			imgui_trinex_setup_render_state(ctx, draw_data);

			int global_idx_offset = 0;
			int global_vtx_offset = 0;
			ImVec2 clip_off       = draw_data->DisplayPos;

			ctx->bind_vertex_attribute(RHIVertexSemantic::TexCoord, 0, 0, offsetof(ImDrawVert, pos));
			ctx->bind_vertex_attribute(RHIVertexSemantic::TexCoord, 1, 0, offsetof(ImDrawVert, uv));
			ctx->bind_vertex_attribute(RHIVertexSemantic::Color, 0, 0, offsetof(ImDrawVert, col));

			ctx->bind_vertex_buffer(vd->vertex_buffer, 0, sizeof(ImDrawVert), 0);
			ctx->bind_index_buffer(vd->index_buffer, RHIIndexFormat::UInt16);

			ImGuiPipeline::instance()->push(ctx);

			trinex_rhi_pop_stage(ctx);
			{
				trinex_profile_cpu_n("Render");

				auto pipeline = ImGuiPipeline::instance();
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					trinex_rhi_push_stage(ctx, "ImGui Command list");
					const ImDrawList* cmd_list = draw_data->CmdLists[n];

					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
						trinex_rhi_push_stage(ctx, "ImGui Draw Command");
						if (pcmd->UserCallback != nullptr)
						{
							if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
								imgui_trinex_setup_render_state(ctx, draw_data);
							else
								pcmd->UserCallback(cmd_list, pcmd);
						}
						else
						{
							ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
							ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);

							if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								continue;

							RHIScissors scissor;
							scissor.pos.x  = clip_min.x;
							scissor.pos.y  = clip_min.y;
							scissor.size.x = (clip_max.x - clip_min.x);
							scissor.size.y = (clip_max.y - clip_min.y);

							ctx->scissor(scissor);

							if (!pipeline->srv)
							{
								pipeline->srv = pcmd->TextureId.texture ? pcmd->TextureId.texture->rhi_srv()
								                                        : pcmd->TextureId.surface->rhi_srv();
							}

							{
								trinex_profile_cpu_n("Drawing");
								pipeline->apply(ctx, bd->sampler);

								ctx->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset,
								                  pcmd->VtxOffset + global_vtx_offset);
							}
						}

						trinex_rhi_pop_stage(ctx);
					}

					global_idx_offset += cmd_list->IdxBuffer.Size;
					global_vtx_offset += cmd_list->VtxBuffer.Size;
					trinex_rhi_pop_stage(ctx);
				}
			}
			trinex_rhi_pop_stage(ctx);
			ImGuiPipeline::instance()->pop(ctx);
		}

		static void imgui_trinex_create_fonts_texture()
		{
			// Build texture atlas
			ImGuiIO& io         = ImGui::GetIO();
			ImGuiTrinexData* bd = imgui_trinex_backend_data();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

			bd->font_texture = Object::new_instance<EngineResource<Texture2D>>(
			        Strings::format("FontsTexture {}", reinterpret_cast<size_t>(ImGui::GetCurrentContext())));
			bd->font_texture->format = RHIColorFormat::R8G8B8A8;
			bd->font_texture->mips.emplace_back();
			auto& mip = bd->font_texture->mips[0];
			mip.size  = {width, height};
			mip.data  = Buffer(pixels, pixels + width * height * 4);
			bd->font_texture->init_render_resources();

			auto package = Package::static_find_package("TrinexEditor::ImGui", true);
			package->add_object(bd->font_texture);

			bd->sampler = Sampler(RHISamplerFilter::Bilinear);

			// Store our identifier
			io.Fonts->SetTexID(bd->font_texture);
		}

		static void imgui_trinex_destroy_device_objects()
		{
			ImGuiTrinexData* bd         = imgui_trinex_backend_data();
			bool call_garbage_collector = !Engine::engine_instance->is_shuting_down();

			if (bd->font_texture)
			{
				if (call_garbage_collector)
					GarbageCollector::destroy(bd->font_texture);

				ImGui::GetIO().Fonts->SetTexID(ImTextureID());
				bd->font_texture = {};
			}
		}

		static void imgui_trinex_create_device_objects()
		{
			ImGuiTrinexData* bd = imgui_trinex_backend_data();
			if (bd->font_texture)
				imgui_trinex_destroy_device_objects();

			ImGuiPipeline::create();
			imgui_trinex_create_fonts_texture();
		}


		static void imgui_trinex_create_window(ImGuiViewport* viewport)
		{
			ImGuiTrinexViewportData* vd = IM_NEW(ImGuiTrinexViewportData)();
			viewport->RendererUserData  = vd;
		}

		static void imgui_trinex_destroy_window(ImGuiViewport* viewport)
		{
			if (ImGuiTrinexViewportData* vd = (ImGuiTrinexViewportData*) viewport->RendererUserData)
			{
				render_thread()->wait();
				IM_DELETE(vd);
			}
			viewport->RendererUserData = nullptr;
		}

		static void imgui_trinex_init_platform_interface()
		{
			ImGuiPlatformIO& platform_io       = ImGui::GetPlatformIO();
			platform_io.Renderer_CreateWindow  = imgui_trinex_create_window;
			platform_io.Renderer_DestroyWindow = imgui_trinex_destroy_window;
		}

		static void imgui_trinex_shutdown_platform_interface()
		{
			// Do not destroy from render thread!
		}

		bool imgui_trinex_rhi_init(Engine::Window* window, ImGuiContext* ctx)
		{
			ImGui::SetCurrentContext(ctx);
			ImGuiIO& io = ImGui::GetIO();
			IMGUI_CHECKVERSION();
			IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

			ImGuiTrinexData* bd = IM_NEW(ImGuiTrinexData)();
			bd->window          = window->render_viewport();

			io.BackendRendererUserData = (void*) bd;
			io.BackendRendererName     = "imgui_impl_trinex";
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				imgui_trinex_init_platform_interface();

			imgui_trinex_create_device_objects();

			ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
			main_viewport->RendererUserData = IM_NEW(ImGuiTrinexViewportData)();
			return true;
		}

		void imgui_trinex_rhi_shutdown(ImGuiContext* ctx)
		{
			ImGui::SetCurrentContext(ctx);
			ImGuiTrinexData* bd = imgui_trinex_backend_data();
			IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
			ImGuiIO& io = ImGui::GetIO();

			ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			if (auto vd = reinterpret_cast<ImGuiTrinexViewportData*>(main_viewport->RendererUserData))
				IM_DELETE(vd);
			main_viewport->RendererUserData = nullptr;

			imgui_trinex_shutdown_platform_interface();
			imgui_trinex_destroy_device_objects();
			io.BackendRendererName     = nullptr;
			io.BackendRendererUserData = nullptr;
			io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasViewports);
			IM_DELETE(bd);
		}
	}// namespace ImGuiBackend_RHI

	//////////////////////////// IMGUI RHI IMPLEMENTATION END ////////////////////////////


	//////////////////////////// IMGUI WINDOW BACKEND IMPLEMENTATION ////////////////////////////

	namespace ImGuiBackend_Window
	{
		static Map<Engine::Window*, ImGuiWindow*> m_window_map;

		struct ImGuiTrinexWindowData {
			Window* window;
			float time;
			bool update_monitors;

			ImGuiTrinexWindowData() : window(nullptr), time(engine_instance->time_seconds()), update_monitors(true) {}
		};

		class ImGuiViewportClient : public ViewportClient
		{
			trinex_declare_class(ImGuiViewportClient, ViewportClient);
			ImGuiDrawData m_draw_data;
			class ImGuiWindow* m_window;

		public:
			ImGuiViewport* viewport = nullptr;
			ImGuiContext* context;

			ViewportClient& on_bind_viewport(class RenderViewport* viewport) override
			{
				m_window = ImGuiWindow::current();
				return *this;
			}

			ViewportClient& render_frame(RenderViewport* viewport)
			{
				auto bd = ImGuiBackend_RHI::imgui_trinex_backend_data();
				std::swap(viewport, bd->window);// Temporary set as main viewport

				RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
				{
					trinex_rhi_push_stage(ctx, "ImGuiViewportClient");
					ImGuiContextLock lock(m_window->context());
					ImGuiBackend_RHI::imgui_trinex_rhi_render_draw_data(ctx, m_draw_data.draw_data());
				}
				RHIContextPool::global_instance()->end_context(ctx);

				m_draw_data.swap_render_index();
				trinex_rhi_pop_stage(ctx);

				std::swap(viewport, bd->window);// Restore main viewport

				viewport->rhi_present();
				return *this;
			}

			ViewportClient& update(class RenderViewport* vp, float dt) override
			{
				m_draw_data.copy(viewport->DrawData);
				m_draw_data.swap_logic_index();
				render_thread()->call([self = Pointer(this), viewport = Pointer(vp)]() { self->render_frame(viewport); });
				return *this;
			}
		};

		static ImGuiTrinexWindowData* imgui_trinex_backend_data()
		{
			return ImGui::GetCurrentContext() ? reinterpret_cast<ImGuiTrinexWindowData*>(ImGui::GetIO().BackendPlatformUserData)
			                                  : nullptr;
		}

		static FORCE_INLINE Engine::Window* window_from(const Event& event)
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

#define IMGUI_EVENT_FUNC_HEADER(return_value)                                                                                    \
	ImGuiContext* context         = nullptr;                                                                                     \
	Engine::Window* engine_window = window_from(event);                                                                          \
	{                                                                                                                            \
		ImGuiWindow* window = nullptr;                                                                                           \
                                                                                                                                 \
		if (engine_window)                                                                                                       \
		{                                                                                                                        \
			try                                                                                                                  \
			{                                                                                                                    \
				window = m_window_map.at(engine_window);                                                                         \
			}                                                                                                                    \
			catch (...)                                                                                                          \
			{                                                                                                                    \
				window = nullptr;                                                                                                \
			}                                                                                                                    \
		}                                                                                                                        \
		else                                                                                                                     \
		{                                                                                                                        \
			return return_value;                                                                                                 \
		}                                                                                                                        \
                                                                                                                                 \
		if (window == nullptr || (context = window->context()) == nullptr)                                                       \
		{                                                                                                                        \
			if (auto render_viewport = engine_window->render_viewport())                                                         \
			{                                                                                                                    \
				if (ImGuiViewportClient* client = Object::instance_cast<ImGuiViewportClient>(render_viewport->client()))         \
				{                                                                                                                \
					context = client->context;                                                                                   \
				}                                                                                                                \
			}                                                                                                                    \
		}                                                                                                                        \
		if (context == nullptr)                                                                                                  \
		{                                                                                                                        \
			return return_value;                                                                                                 \
		}                                                                                                                        \
	}                                                                                                                            \
	ImGuiContextSaver imgui_context_saver(context);

		static void imgui_sent_mouse_position(Engine::Window* engine_window, float x, float y)
		{
			auto& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				auto info = Platform::monitor_info(engine_window->monitor_index());
				auto pos  = engine_window->position();
				io.AddMousePosEvent(x + pos.x, info.size.y - (y + pos.y));
			}
			else
			{
				io.AddMousePosEvent(x, engine_window->size().y - y);
			}
		}

		static void on_mouse_move(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data = event.mouse.motion;
			auto& io   = ImGui::GetIO();
			io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);

			imgui_sent_mouse_position(engine_window, data.x, data.y);
		}

		static void on_mouse_button(const Event& event, bool is_pressed)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data        = event.mouse.button;
			auto& io          = ImGui::GetIO();
			auto imgui_button = imgui_button_of(data.button);
			if (imgui_button != -1)
			{
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(imgui_button, is_pressed);
			}
		}

		static void on_mouse_wheel(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data = event.mouse.wheel;
			auto& io   = ImGui::GetIO();
			io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
			io.AddMouseWheelEvent(data.x, data.y);
		}

		static void on_keyboard_button(const Event& event, bool is_pressed)
		{
			IMGUI_EVENT_FUNC_HEADER();

			auto& data        = event.keyboard;
			auto& io          = ImGui::GetIO();
			auto imgui_button = imgui_button_of(data.key);

			if (imgui_button != ImGuiKey_None)
			{
				if (is_in<ImGuiKey_LeftCtrl, ImGuiKey_RightCtrl>(imgui_button))
					io.AddKeyEvent(ImGuiMod_Ctrl, is_pressed);
				if (is_in<ImGuiKey_LeftShift, ImGuiKey_RightShift>(imgui_button))
					io.AddKeyEvent(ImGuiMod_Shift, is_pressed);
				if (is_in<ImGuiKey_LeftAlt, ImGuiKey_RightAlt>(imgui_button))
					io.AddKeyEvent(ImGuiMod_Alt, is_pressed);
				if (is_in<ImGuiKey_LeftSuper, ImGuiKey_RightSuper>(imgui_button))
					io.AddKeyEvent(ImGuiMod_Super, is_pressed);
				io.AddKeyEvent(imgui_button, is_pressed);
			}
		}

		static void on_text_input(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();
			auto& data = event.text_input;
			auto& io   = ImGui::GetIO();

			io.AddInputCharactersUTF8(data.text);
		}

		static void on_window_close(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestClose = true;
			}
		}

		static void on_window_move(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestMove = true;
			}
		}

		static void on_window_resize(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			if (auto* vp = ImGui::FindViewportByPlatformHandle(engine_window))
			{
				vp->PlatformRequestResize = true;
			}
		}

		static void on_finger_down(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			const auto& data = event.touchscreen.finger;
			if (data.index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
				io.AddMouseButtonEvent(0, true);
			}
		}

		static void on_finger_up(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();
			const auto& data = event.touchscreen.finger;
			if (data.index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
				io.AddMouseButtonEvent(0, false);
			}
		}

		static void on_finger_motion(const Event& event)
		{
			IMGUI_EVENT_FUNC_HEADER();

			const auto& data = event.touchscreen.finger_motion;
			if (data.index == 0)
			{
				auto& io = ImGui::GetIO();
				io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
				imgui_sent_mouse_position(engine_window, data.x, data.y);
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

		static FORCE_INLINE Engine::Window* window_from(ImGuiViewport* vp)
		{
			return reinterpret_cast<Engine::Window*>(vp->PlatformHandle);
		}

		static void imgui_trinex_window_create(ImGuiViewport* vp)
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
			auto new_window    = Engine::WindowManager::instance()->create_window(config, parent_window);

			auto render_viewport = new_window->render_viewport();
			auto client          = Object::new_instance<ImGuiViewportClient>();
			client->viewport     = vp;
			client->context      = ImGui::GetCurrentContext();
			render_viewport->client(client);

			vp->PlatformHandle   = new_window;
			vp->PlatformUserData = reinterpret_cast<void*>(new_window->id());

			new_window->on_destroy.push([vp](Window* window) {
				vp->PlatformHandle   = nullptr;
				vp->PlatformUserData = nullptr;
			});
		}


		static void imgui_trinex_window_destroy(ImGuiViewport* vp)
		{
			if (is_in_logic_thread())
			{
				if (vp->ParentViewportId != 0)
				{
					Identifier id = reinterpret_cast<Identifier>(vp->PlatformUserData);
					if (Engine::Window* window = WindowManager::instance()->find(id))
					{
						WindowManager::instance()->destroy_window(window);
					}
				}

				vp->PlatformUserData = vp->PlatformHandle = nullptr;
			}
		}

		static void imgui_trinex_window_show(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				wd->show();
			}
		}

		static void imgui_trinex_set_window_pos(ImGuiViewport* vp, ImVec2 pos)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				auto info = Platform::monitor_info(wd->monitor_index());
				wd->position({pos.x, info.size.y - (pos.y + wd->size().y)});
			}
		}

		static ImVec2 imgui_trinex_get_window_pos(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				auto pos    = wd->position();
				auto info   = Platform::monitor_info(wd->monitor_index());
				float new_y = -pos.y + info.size.y - wd->size().y;
				return {static_cast<float>(pos.x), static_cast<float>(new_y)};
			}

			return {0, 0};
		}

		static void imgui_trinex_set_window_size(ImGuiViewport* vp, ImVec2 size)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				wd->size({size.x, size.y});
			}
		}

		static ImVec2 imgui_trinex_get_window_size(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				auto size = wd->size();
				return {static_cast<float>(size.x), static_cast<float>(size.y)};
			}

			return {0, 0};
		}

		static bool imgui_trinex_get_window_focus(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				return wd->focused();
			}
			return false;
		}

		static void imgui_trinex_set_window_focus(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				wd->focus();
			}
		}

		static void imgui_trinex_set_window_title(ImGuiViewport* vp, const char* title)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				wd->title(title);
			}
		}

		static bool imgui_trinex_get_window_minimized(ImGuiViewport* vp)
		{
			if (Engine::Window* wd = window_from(vp))
			{
				return wd->is_iconify();
			}
			return false;
		}

		static void imgui_trinex_window_init_platform_interface(Engine::Window* window)
		{
			ImGuiPlatformIO& platform_io            = ImGui::GetPlatformIO();
			platform_io.Platform_CreateWindow       = imgui_trinex_window_create;
			platform_io.Platform_DestroyWindow      = imgui_trinex_window_destroy;
			platform_io.Platform_ShowWindow         = imgui_trinex_window_show;
			platform_io.Platform_SetWindowPos       = imgui_trinex_set_window_pos;
			platform_io.Platform_GetWindowPos       = imgui_trinex_get_window_pos;
			platform_io.Platform_SetWindowSize      = imgui_trinex_set_window_size;
			platform_io.Platform_GetWindowSize      = imgui_trinex_get_window_size;
			platform_io.Platform_SetWindowFocus     = imgui_trinex_set_window_focus;
			platform_io.Platform_GetWindowFocus     = imgui_trinex_get_window_focus;
			platform_io.Platform_GetWindowMinimized = imgui_trinex_get_window_minimized;
			platform_io.Platform_SetWindowTitle     = imgui_trinex_set_window_title;

			ImGuiViewport* main_viewport    = ImGui::GetMainViewport();
			main_viewport->PlatformHandle   = window;
			main_viewport->PlatformUserData = reinterpret_cast<void*>(window->id());
		}

		static void imgui_trinex_window_init(Engine::Window* window)
		{
			ImGuiIO& io            = ImGui::GetIO();
			io.BackendPlatformName = "imgui_impl_trinex";
			auto bd                = IM_NEW(ImGuiTrinexWindowData)();

			enable_events();
			io.BackendPlatformUserData = bd;
			io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;


			if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
				imgui_trinex_window_init_platform_interface(window);
		}

		static void imgui_trinex_window_shutdown()
		{
			ImGui::DestroyPlatformWindows();

			ImGuiIO& io            = ImGui::GetIO();
			io.BackendPlatformName = nullptr;
			IM_DELETE(reinterpret_cast<ImGuiTrinexWindowData*>(io.BackendPlatformUserData));
			io.BackendPlatformUserData = nullptr;
		}

		static void imgui_trinex_window_update_monitors()
		{
			auto* bd                     = imgui_trinex_backend_data();
			ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
			platform_io.Monitors.resize(0);
			bd->update_monitors = false;

			size_t display_count = Platform::monitors_count();
			for (size_t n = 0; n < display_count; n++)
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

		static void imgui_trinex_window_new_frame(Engine::Window* window)
		{
			ImGuiIO& io = ImGui::GetIO();
			auto bd     = imgui_trinex_backend_data();

			Vector2f size          = Vector2f(window->size());
			Vector2f drawable_size = Vector2f(window->size());

			io.DisplaySize = ImVec2(size.x, size.y);
			if (drawable_size.x > 0 && drawable_size.y > 0)
				io.DisplayFramebufferScale = ImVec2(drawable_size.x / size.x, drawable_size.y / size.y);

			// Update monitors
			if (bd->update_monitors)
				imgui_trinex_window_update_monitors();

			float current_time = engine_instance->time_seconds();
			io.DeltaTime       = bd->time > 0.0 ? (current_time - bd->time) : (1.0f / 60.0f);
			bd->time           = current_time;
		}
	}// namespace ImGuiBackend_Window

	ImDrawData* ImGuiDrawData::draw_data()
	{
		return &m_draw_data[m_render_index];
	}

	static void release_draw_data(ImDrawData& data)
	{
		if (data.CmdListsCount > 0)
		{
			for (int index = 0; index < data.CmdListsCount; index++)
			{
				ImDrawList* drawList = data.CmdLists[index];
				IM_DELETE(drawList);
			}

			data.CmdLists.clear();
		}

		data.Clear();
	}

	ImGuiDrawData& ImGuiDrawData::release(bool full)
	{

		release_draw_data(m_draw_data[m_logic_index]);
		if (full)
			release_draw_data(m_draw_data[m_render_index]);

		return *this;
	}

	ImGuiDrawData& ImGuiDrawData::copy(ImDrawData* draw_data)
	{
		release(false);

		m_draw_data[m_logic_index] = *draw_data;

		m_draw_data[m_logic_index].CmdLists.resize(draw_data->CmdListsCount);
		for (int index = 0; index < draw_data->CmdListsCount; index++)
		{
			ImDrawList* drawList                       = draw_data->CmdLists[index]->CloneOutput();
			m_draw_data[m_logic_index].CmdLists[index] = drawList;
		}

		return *this;
	}

	ImGuiDrawData& ImGuiDrawData::swap_render_index()
	{
		m_render_index = (m_render_index + 1) % 2;
		return *this;
	}

	ImGuiDrawData& ImGuiDrawData::swap_logic_index()
	{
		m_logic_index = (m_logic_index + 1) % 2;
		return *this;
	}

	ImGuiDrawData::~ImGuiDrawData()
	{
		release(true);
	}

	ImGuiWidget::ImGuiWidget() {}

	void ImGuiWidget::init(class RenderViewport* viewport) {}

	ImGuiWidgetsList::Node* ImGuiWidgetsList::close_window_internal(Node* node)
	{
		node->widget->on_close();
		return destroy(node);
	}

	ImGuiWidgetsList& ImGuiWidgetsList::close_widget(class ImGuiWidget* widget)
	{
		Node* node = m_root;

		while (node && node->widget != widget)
		{
			node = node->next;
		}

		if (node)
		{
			close_window_internal(node);
		}
		return *this;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::close_all_widgets()
	{
		Node* node = m_root;
		while (node)
		{
			node = close_window_internal(node);
		}
		m_root = nullptr;
		return *this;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::render(class RenderViewport* viewport)
	{
		Node* node = m_root;

		while (node)
		{
			bool status = node->widget->render(viewport);
			node->widget->frame_number += 1;
			if (status)
			{
				node = node->next;
			}
			else
			{
				node = close_window_internal(node);
			}
		}

		return *this;
	}

	ImGuiWidgetsList::Node* ImGuiWidgetsList::destroy(Node* node)
	{
		if (node == m_root)
		{
			m_root = m_root->next;
		}

		if (node->parent)
		{
			node->parent->next = node->next;
		}

		if (node->next)
		{
			node->next->parent = node->parent;
		}

		trx_delete node->widget;

		Node* next = node->next;
		trx_delete node;

		return next;
	}

	ImGuiWidgetsList& ImGuiWidgetsList::push(ImGuiWidget* widget, const void* id)
	{
		widget->init(ImGuiWindow::current()->window()->render_viewport());
		Node* parent_node = m_root;
		while (parent_node && parent_node->next) parent_node = parent_node->next;

		Node* node   = trx_new Node();
		node->widget = widget;
		node->parent = parent_node;
		node->next   = nullptr;
		node->id     = id;

		if (parent_node)
		{
			parent_node->next = node;
		}

		if (m_root == nullptr)
		{
			m_root = node;
		}

		return *this;
	}

	ImGuiWidgetsList::~ImGuiWidgetsList()
	{
		while (m_root)
		{
			destroy(m_root);
		}
	}

	trinex_implement_engine_class_default_init(ImGuiWindow, 0);

	static ImGuiWindow* m_current_window = nullptr;

	bool ImGuiWindow::initialize(Engine::Window* window, ImGuiContext* context)
	{
		m_window                                    = window;
		m_context                                   = context;
		ImGuiBackend_Window::m_window_map[m_window] = this;

		ImGuiContextLock lock(context);

		ImGuiBackend_Window::imgui_trinex_window_init(window);
		ImGuiBackend_RHI::imgui_trinex_rhi_init(window, context);
		return true;
	}

	bool ImGuiWindow::terminate()
	{
		ImGuiWindow* current_window = ImGuiWindow::current();

		if (this == current_window)
			current_window = nullptr;

		free_resources();

		ImGuiContextLock lock(m_context);

		ImGuiBackend_RHI::imgui_trinex_rhi_shutdown(m_context);
		ImGuiBackend_Window::imgui_trinex_window_shutdown();

		ImGuiBackend_Window::m_window_map.erase(m_window);

		m_window  = nullptr;
		m_context = nullptr;

		ImGuiWindow::make_current(current_window);
		return true;
	}

	ImGuiWindow& ImGuiWindow::free_resources()
	{
		on_destroy();
		return *this;
	}

	ImGuiContext* ImGuiWindow::context() const
	{
		return m_context;
	}

	ImDrawData* ImGuiWindow::draw_data()
	{
		return m_draw_data.draw_data();
	}

	void ImGuiWindow::make_current(ImGuiWindow* window)
	{
		if (m_current_window == window)
			return;

		m_current_window = window;

		if (m_current_window)
		{
			ImGui::SetCurrentContext(window->context());
		}
		else
		{
			ImGui::SetCurrentContext(nullptr);
		}
	}


	ImGuiWindow& ImGuiWindow::new_frame()
	{
		make_current(this);
		ImGuiBackend_Window::imgui_trinex_window_new_frame(m_window);
		ImGui::NewFrame();
		++m_frame;
		return *this;
	}

	ImGuiWindow& ImGuiWindow::end_frame()
	{
		make_current(this);

		widgets.render(m_window->render_viewport());
		ImGui::Render();

		m_draw_data.copy(ImGui::GetDrawData());
		m_draw_data.swap_logic_index();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
		}

		render_thread()->call([this]() {
			auto viewport = window()->render_viewport();

			RHIContext* ctx = RHIContextPool::global_instance()->begin_context();
			{
				ctx->viewport(RHIViewport(viewport->size()));
				ctx->scissor(RHIScissors(viewport->size()));
				auto rtv = viewport->rhi_rtv();
				ctx->barrier(viewport->rhi_texture(), RHIAccess::TransferDst);
				ctx->clear_rtv(rtv, 0.f, 0.f, 0.f, 1.f).bind_render_target1(rtv);

				ImGuiContextLock lock(m_context);
				ImGuiBackend_RHI::imgui_trinex_rhi_render_draw_data(ctx, draw_data());
			}
			RHIContextPool::global_instance()->end_context(ctx);

			viewport->rhi_present();
			m_draw_data.swap_render_index();
		});

		make_current(nullptr);

		return *this;
	}

	Engine::Window* ImGuiWindow::window() const
	{
		return m_window;
	}

	size_t ImGuiWindow::frame_index() const
	{
		return m_frame;
	}

	ImGuiWindow* ImGuiWindow::current()
	{
		return m_current_window;
	}
}// namespace Engine

namespace ImGui
{
	struct InputTextCallback {
		ImGuiInputTextCallback callback;
		void* userdata;
		Engine::String* str;
	};

	static int input_text_callback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback* userdata = reinterpret_cast<InputTextCallback*>(data->UserData);

		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			Engine::String* str = userdata->str;
			const int new_size  = data->BufTextLen;
			str->resize(new_size);
			data->Buf = str->data();
		}

		if (userdata->callback)
		{
			data->UserData = userdata->userdata;
			return userdata->callback(data);
		}

		return 0;
	}

	void TextEllipsis(const char* text, float max_width)
	{
		if (!text || max_width <= 0.0f)
			return;

		const char* end_ptr   = text + strlen(text);
		float text_full_width = ImGui::CalcTextSize(text, end_ptr).x;

		if (text_full_width <= max_width)
		{
			ImGui::TextUnformatted(text);
			return;
		}

		const char* ellipsis = "...";
		float ellipsis_width = ImGui::CalcTextSize(ellipsis).x;
		const char* cut_ptr  = text;

		while (cut_ptr < end_ptr)
		{
			float current_width = ImGui::CalcTextSize(text, cut_ptr + 1).x;
			if (current_width + ellipsis_width > max_width)
				break;
			++cut_ptr;
		}

		ImGui::TextUnformatted(text, cut_ptr);
		ImGui::SameLine(0, 0);
		ImGui::TextUnformatted(ellipsis);
	}

	bool InputText(const char* label, Engine::String& buffer, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback,
	               void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputText(label, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool InputTextMultiline(const char* label, Engine::String& buffer, const ImVec2& size, ImGuiInputTextFlags flags,
	                        ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextMultiline(label, buffer.data(), buffer.size() + 1, size, flags, input_text_callback, &data);
	}

	bool InputTextWithHint(const char* label, const char* hint, Engine::String& buffer, ImGuiInputTextFlags flags,
	                       ImGuiInputTextCallback callback, void* user_data)
	{
		InputTextCallback data;
		data.callback = callback;
		data.userdata = user_data;
		data.str      = &buffer;

		flags |= ImGuiInputTextFlags_CallbackResize;
		return ImGui::InputTextWithHint(label, hint, buffer.data(), buffer.size() + 1, flags, input_text_callback, &data);
	}

	bool ImageButton(ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1,
	                 const ImVec4& bg_col, const ImVec4& tint_col)
	{
		ImGuiContext& g     = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return false;

		return ImageButtonEx(window->GetID(static_cast<const void*>(user_texture_id)), user_texture_id, image_size, uv0, uv1,
		                     bg_col, tint_col);
	}

	float TableGetAutoWidth(const char* name)
	{
		return TableGetAutoWidth(ImGui::GetID(name));
	}

	float TableGetAutoWidth(ImGuiID table_id)
	{
		ImGuiContext& g   = *GImGui;
		ImGuiTable* table = g.Tables.GetByKey(table_id);

		if (table == nullptr)
			return 0.f;

		const ImGuiStyle& style = g.Style;
		const int columnCount   = table->ColumnsCount;

		float width = 0.f;

		if (columnCount > 0)
		{
			width = table->Columns[columnCount - 1].WidthAuto;

			for (int i = 0, end = columnCount - 1; i < end; ++i)
			{
				width += table->Columns[i].WidthGiven;
			}
		}

		width += static_cast<float>(columnCount) * style.CellPadding.x * 2.0f;

		float spacing = (table->Flags & ImGuiTableFlags_BordersInnerV) ? table->CellSpacingX2 : table->CellSpacingX1;
		if (columnCount > 1)
			width += columnCount * spacing;

		width += table->OuterPaddingX * 2.0f;

		if (table->Flags & ImGuiTableFlags_BordersInnerV)
			width += (columnCount - 1) * style.FrameBorderSize;

		if (table->Flags & ImGuiTableFlags_BordersOuterV)
			width += 2.0f * style.FrameBorderSize;

		return width;
	}
}// namespace ImGui


namespace Engine
{
	trinex_implement_class_default_init(Engine::ImGuiBackend_Window::ImGuiViewportClient, 0);
}// namespace Engine
