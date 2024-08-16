#define USE_RHI_IMPLEMENTATION 0

#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/package.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_material.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine::ImGuiBackend
{
    class ImGuiMaterial : public ShaderMaterial
    {
        declare_class(ImGuiMaterial, ShaderMaterial);

    public:
        ImGuiMaterial& postload() override
        {
            auto shader = pipeline->vertex_shader();

            for (auto& attribute : shader->attributes)
            {
                attribute.stream_index = 0;

                if (attribute.name == "pos")
                {
                    attribute.offset = offsetof(ImDrawVert, pos);
                }
                else if (attribute.name == "col")
                {
                    attribute.offset = offsetof(ImDrawVert, col);
                }
                else if (attribute.name == "uv")
                {
                    attribute.offset = offsetof(ImDrawVert, uv);
                }
            }

            Super::postload();
            return *this;
        }
    };

    implement_class_default_init(Engine::ImGuiBackend, ImGuiMaterial, Class::IsAsset);

#if !USE_RHI_IMPLEMENTATION
    bool imgui_trinex_rhi_init(ImGuiContext* ctx);
    void imgui_trinex_rhi_shutdown(ImGuiContext* ctx);
    void imgui_trinex_rhi_render_draw_data(ImGuiContext* ctx, ImDrawData* draw_data);

    class ImGuiVertexBuffer : public DynamicVertexBuffer
    {
    public:
        int m_size = 0;

        const byte* data() const override
        {
            return nullptr;
        }

        size_t size() const override
        {
            return static_cast<size_t>(m_size) * element_size();
        }

        size_t element_size() const override
        {
            return sizeof(ImDrawVert);
        }
    };

    class ImGuiIndexBuffer : public DynamicIndexBuffer
    {
    public:
        int m_size = 0;

        const byte* data() const override
        {
            return nullptr;
        }

        size_t size() const override
        {
            return static_cast<size_t>(m_size) * element_size();
        }

        size_t element_size() const override
        {
            return sizeof(ImDrawIdx);
        }
    };

    struct ImGuiTrinexData {
        ImTextureID font_texture;
        Material* material;
        CombinedImageSampler2DMaterialParameter* texture_parameter;
        Mat4MaterialParameter* model_parameter;

        ImGuiTrinexData()
        {
            memset((void*) this, 0, sizeof(*this));
        }
    };

    struct ImGuiTrinexViewportData {
        ImGuiVertexBuffer* vertex_buffer;
        ImGuiIndexBuffer* index_buffer;

        ImGuiTrinexViewportData()
        {
            vertex_buffer = Object::new_instance<EngineResource<ImGuiVertexBuffer>>();
            index_buffer  = Object::new_instance<EngineResource<ImGuiIndexBuffer>>();
        }

        ~ImGuiTrinexViewportData()
        {
            bool call_garbage_collector = !engine_instance->is_shuting_down();
            if (index_buffer)
            {
                if (call_garbage_collector)
                {
                    GarbageCollector::destroy(index_buffer);
                }
                index_buffer = nullptr;
            }

            if (vertex_buffer)
            {
                if (call_garbage_collector)
                {
                    GarbageCollector::destroy(vertex_buffer);
                }
                vertex_buffer = nullptr;
            }
        }
    };

    static ImGuiTrinexData* imgui_trinex_backend_data()
    {
        return ImGui::GetCurrentContext() ? (ImGuiTrinexData*) ImGui::GetIO().BackendRendererUserData : nullptr;
    }

    // Functions

    static void imgui_trinex_setup_render_state(ImDrawData* draw_data)
    {
        ImGuiTrinexData* bd = imgui_trinex_backend_data();

        ViewPort viewport;
        viewport.size = {draw_data->DisplaySize.x * ImGuiRenderer::rhi_rendering_scale_factor,
                         draw_data->DisplaySize.y * ImGuiRenderer::rhi_rendering_scale_factor};
        rhi->viewport(viewport);

        float L                        = draw_data->DisplayPos.x;
        float R                        = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
        float T                        = draw_data->DisplayPos.y;
        float B                        = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
        bd->model_parameter->param     = glm::ortho(L, R, B, T);
        bd->texture_parameter->texture = nullptr;
    }

#endif

    // Render function
    void imgui_trinex_rhi_render_draw_data(ImGuiContext* ctx, ImDrawData* draw_data)
    {
#if USE_RHI_IMPLEMENTATION
        rhi->imgui_render(ctx, draw_data);
#else
        ImGui::SetCurrentContext(ctx);
        rhi->push_debug_stage("ImGui Render");

        // Avoid rendering when minimized
        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
            return;

        ImGuiTrinexData* bd         = imgui_trinex_backend_data();
        ImGuiTrinexViewportData* vd = reinterpret_cast<ImGuiTrinexViewportData*>(draw_data->OwnerViewport->RendererUserData);

        if (bd == nullptr || vd == nullptr)
            return;

        rhi->push_debug_stage("ImGui Setup state");
        const float fb_height          = draw_data->DisplaySize.y * draw_data->FramebufferScale.y;
        const ViewPort backup_viewport = rhi->viewport();
        const Scissor backup_scissor   = rhi->scissor();

        if (!vd->vertex_buffer || vd->vertex_buffer->m_size < draw_data->TotalVtxCount)
        {
            vd->vertex_buffer->m_size = draw_data->TotalVtxCount + 5000;
            vd->vertex_buffer->rhi_create();
        }

        if (!vd->index_buffer || vd->index_buffer->m_size < draw_data->TotalIdxCount)
        {
            vd->index_buffer->m_size = draw_data->TotalIdxCount + 10000;
            vd->index_buffer->rhi_create();
        }

        // Upload vertex/index data into a single contiguous GPU buffer

        size_t vtx_offset = 0;
        size_t idx_offset = 0;

        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            size_t vtx_size            = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            size_t idx_size            = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

            vd->vertex_buffer->rhi_update(vtx_offset, vtx_size, reinterpret_cast<const byte*>(cmd_list->VtxBuffer.Data));
            vd->index_buffer->rhi_update(idx_offset, idx_size, reinterpret_cast<const byte*>(cmd_list->IdxBuffer.Data));

            vtx_offset += vtx_size;
            idx_offset += idx_size;
        }

        imgui_trinex_setup_render_state(draw_data);

        int global_idx_offset = 0;
        int global_vtx_offset = 0;
        ImVec2 clip_off       = draw_data->DisplayPos;

        rhi->pop_debug_stage();
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            rhi->push_debug_stage("ImGui Command list");
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
                rhi->push_debug_stage("ImGui Draw Command");
                if (pcmd->UserCallback != nullptr)
                {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        imgui_trinex_setup_render_state(draw_data);
                    else
                        pcmd->UserCallback(cmd_list, pcmd);
                }
                else if (pcmd->UpdateImageCallback)
                {
                    ImTextureID next_texture       = pcmd->UpdateImageCallback(pcmd->UserCallbackData);
                    bd->texture_parameter->texture = next_texture.texture;
                    bd->texture_parameter->sampler = next_texture.sampler;
                }
                else
                {
                    ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
                    ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    Scissor scissor;
                    scissor.pos.x  = clip_min.x * ImGuiRenderer::rhi_rendering_scale_factor;
                    scissor.pos.y  = (fb_height - clip_max.y) * ImGuiRenderer::rhi_rendering_scale_factor;
                    scissor.size.x = (clip_max.x - clip_min.x) * ImGuiRenderer::rhi_rendering_scale_factor;
                    scissor.size.y = (clip_max.y - clip_min.y) * ImGuiRenderer::rhi_rendering_scale_factor;

                    rhi->scissor(scissor);

                    if (!bd->texture_parameter->texture)
                    {
                        bd->texture_parameter->texture = pcmd->TextureId.texture;
                        bd->texture_parameter->sampler = pcmd->TextureId.sampler;
                    }

                    if (bd->texture_parameter->sampler == nullptr)
                    {
                        bd->texture_parameter->sampler = bd->font_texture.sampler;
                    }

                    bd->material->apply();
                    vd->vertex_buffer->rhi_bind(0);
                    vd->index_buffer->rhi_bind();

                    rhi->draw_indexed(pcmd->ElemCount, pcmd->IdxOffset + global_idx_offset, pcmd->VtxOffset + global_vtx_offset);

                    bd->texture_parameter->texture = nullptr;
                    bd->texture_parameter->sampler = nullptr;
                }

                rhi->pop_debug_stage();
            }
            global_idx_offset += cmd_list->IdxBuffer.Size;
            global_vtx_offset += cmd_list->VtxBuffer.Size;
            rhi->pop_debug_stage();
        }

        rhi->viewport(backup_viewport);
        rhi->scissor(backup_scissor);
        rhi->pop_debug_stage();
#endif
    }

#if !USE_RHI_IMPLEMENTATION
    static void imgui_trinex_create_fonts_texture()
    {
        // Build texture atlas
        ImGuiIO& io         = ImGui::GetIO();
        ImGuiTrinexData* bd = imgui_trinex_backend_data();
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        ImTextureID& texture = bd->font_texture;

        texture.texture = Object::new_instance<EngineResource<Texture2D>>(
                Strings::format("FontsTexture {}", reinterpret_cast<size_t>(ImGui::GetCurrentContext())));
        texture.texture->init(ColorFormat::R8G8B8A8, Size2D(static_cast<float>(width), static_cast<float>(height)), pixels,
                              static_cast<size_t>(width * height * 4));
        auto package = Package::static_find_package("Engine::ImGui", true);
        package->add_object(texture.texture);

        texture.sampler = Object::new_instance<EngineResource<Sampler>>(
                Strings::format("Sampler {}", reinterpret_cast<size_t>(ImGui::GetCurrentContext())));
        texture.sampler->filter = SamplerFilter::Trilinear;
        texture.sampler->rhi_create();
        package->add_object(texture.sampler);

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
            {
                GarbageCollector::destroy(bd->font_texture.texture);
                GarbageCollector::destroy(bd->font_texture.sampler);
            }
            ImGui::GetIO().Fonts->SetTexID(0);
            bd->font_texture = {};
        }

        if (bd->material)
        {
            bd->material          = nullptr;
            bd->texture_parameter = nullptr;
            bd->model_parameter   = nullptr;
        }
    }

    static void imgui_trinex_create_device_objects()
    {
        ImGuiTrinexData* bd = imgui_trinex_backend_data();
        if (bd->font_texture)
            imgui_trinex_destroy_device_objects();

        bd->material = DefaultResources::Materials::imgui;
        bd->texture_parameter =
                reinterpret_cast<CombinedImageSampler2DMaterialParameter*>(bd->material->find_parameter(Name::texture));
        bd->model_parameter = reinterpret_cast<Mat4MaterialParameter*>(bd->material->find_parameter(Name::model));

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
#endif

    bool imgui_trinex_rhi_init(ImGuiContext* ctx)
    {
#if USE_RHI_IMPLEMENTATION
        rhi->imgui_init(ctx);
        return true;
#else
        ImGui::SetCurrentContext(ctx);
        ImGuiIO& io = ImGui::GetIO();
        IMGUI_CHECKVERSION();
        IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

        ImGuiTrinexData* bd        = IM_NEW(ImGuiTrinexData)();
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
#endif
    }

    void imgui_trinex_rhi_shutdown(ImGuiContext* ctx)
    {
#if USE_RHI_IMPLEMENTATION
        rhi->imgui_terminate(ctx);
#else
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
#endif
    }

}// namespace Engine::ImGuiBackend
