#include "api_internal.hpp"
#include <Core/base_engine.hpp>
#include <Core/profiler.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Graphics/pipeline_library.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <RHI/context.hpp>
#include <RHI/handles.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>
#include <Window/window.hpp>

namespace Trinex::RenderBackend
{
	namespace
	{
		class ImGuiPipeline : public GlobalPipelineLibrary
		{
			trinex_declare_pipeline(ImGuiPipeline, GlobalPipelineLibrary);

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

				ctx->bind_pipeline(handle());
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

		struct RenderFlags {
			enum Enum : u8
			{
				Undefined     = 0,
				IsInRendering = 1 << 0,
				IsTargetDirty = 1 << 1,
				ClearLayer    = 1 << 2,
			};

			trinex_bitfield_enum_struct(RenderFlags, u8);
		};

		struct ImGuiTrinexData {
			RHIContext* context  = nullptr;
			RHITexture* layer    = nullptr;
			RenderFlags flags    = RenderFlags::Undefined;
			Vector4f clear_color = {0.f, 0.f, 0.f, 0.f};
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

			bd->context->viewport(RHIRegion());

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

		static void begin_rendering(ImGuiTrinexData* bd)
		{
			if (bd->flags & RenderFlags::IsInRendering)
			{
				if (bd->flags.any(RenderFlags::IsTargetDirty || RenderFlags::ClearLayer))
				{
					bd->context->end_rendering();
					bd->flags.remove(RenderFlags::IsInRendering);
				}
			}

			if (!bd->flags.any(RenderFlags::IsInRendering))
			{
				if (bd->layer)
				{
					bd->context->barrier(bd->layer, RHIAccess::RTV);
					RHIRenderingInfo info = RHIRenderingInfo(bd->layer->as_rtv());

					if (bd->flags & RenderFlags::ClearLayer)
					{
						info.colors[0].load  = RHILoadFunc::Clear;
						info.colors[0].color = bd->clear_color;
						bd->flags.remove(RenderFlags::ClearLayer);
					}

					bd->context->begin_rendering(info);
					bd->flags.set(RenderFlags::IsInRendering);
				}
				bd->flags.remove(RenderFlags::IsTargetDirty);
			}
		}

		static void end_rendering(ImGuiTrinexData* bd)
		{
			if (bd->flags & RenderFlags::IsInRendering)
			{
				bd->context->end_rendering();
				bd->flags.remove(RenderFlags::IsInRendering);
			}
		}

		static void render(RHIContext* ctx, ImDrawData* draw_data)
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


			const Vector2u view_size = {draw_data->DisplaySize.x, draw_data->DisplaySize.y};

			bd->context = ctx;
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

				auto pipeline = ImGuiPipeline::instance();
				for (int n = 0; n < draw_data->CmdListsCount; n++)
				{
					trinex_rhi_push_stage(ctx, "ImGui Command list");
					const ImDrawList* cmd_list = draw_data->CmdLists[n];

					for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
					{
						begin_rendering(bd);

						const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

						if (pcmd->UserCallback != nullptr)
						{
							if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
							{
								end_rendering(bd);
								setup_render_state(draw_data);
							}
							else
								pcmd->UserCallback(cmd_list, pcmd);
						}
						else if (bd->flags.all(RenderFlags::IsInRendering))
						{
							ImVec2 clip_min(Math::max(pcmd->ClipRect.x - clip_off.x, 0.f),
							                Math::max(pcmd->ClipRect.y - clip_off.y, 0.f));

							ImVec2 clip_max(Math::max(pcmd->ClipRect.z - clip_off.x, clip_min.x),
							                Math::max(pcmd->ClipRect.w - clip_off.y, clip_min.y));

							if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
								continue;

							RHIRegion scissor;
							scissor.pos.x  = clip_min.x / draw_data->DisplaySize.x;
							scissor.pos.y  = clip_min.y / draw_data->DisplaySize.y;
							scissor.size.x = (clip_max.x - clip_min.x) / draw_data->DisplaySize.x;
							scissor.size.y = (clip_max.y - clip_min.y) / draw_data->DisplaySize.y;

							ctx->scissor(scissor);

							ImTextureID texture = pcmd->GetTexID();

							pipeline->bind(ctx, texture.texture);
							pipeline->bind(ctx, texture.sampler ? texture.sampler : RHIBilinearSampler::static_sampler());

							ctx->draw_indexed(RHITopology::TriangleList, pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset,
							                  pcmd->VtxOffset + global_vtx_offset);
						}
					}

					global_idx_offset += cmd_list->IdxBuffer.Size;
					global_vtx_offset += cmd_list->VtxBuffer.Size;
					trinex_rhi_pop_stage(ctx);
				}

				end_rendering(bd);
			}
			trinex_rhi_pop_stage(ctx);

			bd->context = nullptr;
			bd->layer   = nullptr;
		}

		static void destroy_device_objects()
		{
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

		static class RenderingListener : public UI::ContextListener
		{

		public:
			RenderingListener() : UI::ContextListener(1) {}

			RenderingListener& on_create(UI::Context* context) override
			{
				ImGui::SetCurrentContext(context->context);
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
				return *this;
			}

			RenderingListener& on_destroy(UI::Context* context) override
			{
				ImGui::SetCurrentContext(context->context);
				ImGuiTrinexData* bd = backend_data();
				IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
				ImGuiIO& io = ImGui::GetIO();

				ImGuiViewport* main_viewport = ImGui::GetMainViewport();
				if (auto vd = reinterpret_cast<ImGuiTrinexViewportData*>(main_viewport->RendererUserData))
					IM_DELETE(vd);
				main_viewport->RendererUserData = nullptr;

				destroy_device_objects();
				io.BackendRendererName     = nullptr;
				io.BackendRendererUserData = nullptr;
				io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasViewports);
				IM_DELETE(bd);
				return *this;
			}

			RenderingListener& on_render(UI::Context* context) override
			{
				auto viewport           = context->window->render_viewport();
				RHISwapchain* swapchain = viewport->swapchain();

				RHIContext* ctx = RHIContextPool::global_instance()->begin();
				{
					auto bd      = RenderBackend::backend_data();
					auto texture = swapchain->as_texture();
					auto rtv     = texture->as_rtv();

					bd->layer   = texture;
					bd->context = ctx;
					bd->flags   = RenderFlags::Undefined;

					ctx->barrier(texture, RHIAccess::TransferDst);
					ctx->clear_rtv(rtv, 0.f, 0.f, 0.f, 1.f);
					ctx->barrier(texture, RHIAccess::RTV);

					RenderBackend::render(ctx, ImGui::GetDrawData());
					ctx->barrier(texture, RHIAccess::PresentSrc);
				}

				RHIContextPool::global_instance()->end(ctx, swapchain->acquire_semaphore(), swapchain->present_semaphore());
				RHI::instance()->present(swapchain);

				return *this;
			}
		} s_listener;

	}// namespace
}// namespace Trinex::RenderBackend

namespace Trinex::UI
{
	static void composite_layers(RenderBackend::ImGuiTrinexData* bd, RHITexture* src, LayerOptions* options)
	{
		if (options->composite_mode == LayerCompositeMode::Undefined)
			return;

		bd->context->barrier(src, RHIAccess::SRVGraphics);
		bd->context->scissor(RHIRegion());

		if (options->composite_mode == LayerCompositeMode::Custom)
		{
			if (options->composite_pass)
			{
				LayerCompositeContext ctx;
				ctx.context     = bd->context;
				ctx.destination = bd->layer;
				ctx.source      = src;
				ctx.opacity     = options->opacity;

				bd->context->barrier(src, RHIAccess::SRVGraphics);
				options->composite_pass->execute(ctx);
			}

			return;
		}

		RenderBackend::begin_rendering(bd);

		switch (options->composite_mode)
		{
			case LayerCompositeMode::Copy: bd->context->blending_state(RHIBlendingState::opaque()); break;
			case LayerCompositeMode::Additive: bd->context->blending_state(RHIBlendingState::additive()); break;
			default: bd->context->blending_state(RHIBlendingState::translucent()); break;
		}

		Pipelines::Passthrow::Args args = {
		        .color_scale = {1.f, 1.f, 1.f, options->opacity},
		};

		Pipelines::Passthrow::passthrow(bd->context, src->as_srv(), args);

		if (options->composite_mode != LayerCompositeMode::AlphaBlend)
		{
			bd->context->blending_state(RHIBlendingState::translucent());
		}
	}

	static void push_layer_callback(const ImDrawList* list, const ImDrawCmd* cmd)
	{
		auto context = active_context();
		auto bd      = RenderBackend::backend_data();
		auto options = static_cast<const UI::LayerOptions*>(cmd->UserCallbackData);
		auto pool    = RHITexturePool::global_instance();

		context->stack.push<RHITexture*>(bd->layer);
		bd->layer = pool->acquire(RHISurfaceFormat::RGBA8, bd->layer->size(), RHITextureFlags::ColorAttachment);

		bd->flags |= RenderBackend::RenderFlags::IsTargetDirty;

		if (options->flags & LayerFlags::ClearOnPush)
		{
			bd->flags |= RenderBackend::RenderFlags::ClearLayer;
			bd->clear_color = options->clear_color;
		}

		bd->context->push_debug_stage("Layer Rendering");
	}

	static void pop_layer_callback(const ImDrawList*, const ImDrawCmd* cmd)
	{
		auto context = active_context();
		auto bd      = RenderBackend::backend_data();
		auto options = static_cast<UI::LayerOptions*>(cmd->UserCallbackData);
		auto pool    = RHITexturePool::global_instance();

		RHITexture* layer = bd->layer;

		bd->layer = *context->stack.pop<RHITexture*>();
		bd->flags |= RenderBackend::RenderFlags::IsTargetDirty;

		RenderBackend::end_rendering(bd);
		bd->context->pop_debug_stage();

		if (options->flags & LayerFlags::CompositeOnPop)
		{
			composite_layers(bd, layer, options);
		}

		pool->release(layer);
	}

	bool begin_layer(const LayerOptions& options)
	{
		Context* context = active_context();
		ImDrawList* list = ImGui::GetWindowDrawList();

		if (list == nullptr || context == nullptr)
			return false;

		context->stack.push<LayerOptions>(options);
		list->AddCallback(push_layer_callback, const_cast<LayerOptions*>(&options), sizeof(options));
		return true;
	}

	void end_layer()
	{
		Context* context = active_context();
		ImDrawList* list = ImGui::GetWindowDrawList();

		if (list == nullptr || context == nullptr)
			return;

		LayerOptions* options = context->stack.pop<LayerOptions>();

		list->AddCallback(pop_layer_callback, options, sizeof(LayerOptions));
		list->AddCallback(ImDrawCallback_ResetRenderState);
	}
}// namespace Trinex::UI
