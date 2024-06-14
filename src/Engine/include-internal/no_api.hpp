#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    class NoApi : public RHI
    {
    public:
        void* context() override;
        NoApi& imgui_init(ImGuiContext*) override;
        NoApi& imgui_terminate(ImGuiContext*) override;
        NoApi& imgui_new_frame(ImGuiContext*) override;
        NoApi& imgui_render(ImGuiContext*, ImDrawData*) override;

        NoApi& destroy_object(RHI_Object*) override;

        NoApi& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
        NoApi& draw(size_t vertex_count, size_t vertices_offset) override;
        NoApi& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
        NoApi& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                      size_t instances) override;

        NoApi& begin_render() override;
        NoApi& end_render() override;
        NoApi& wait_idle() override;
        const String& renderer() override;
        const String& name() override;

        void bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;
        void viewport(const ViewPort& viewport) override;
        ViewPort viewport() override;

        RHI_Sampler* create_sampler(const Sampler* sampler) override;
        RHI_Texture* create_texture_2d(const Texture2D*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_Viewport* create_viewport(RenderViewport* vp) override;

        NoApi& push_global_params(const GlobalShaderParameters& params) override;
        NoApi& pop_global_params() override;
        NoApi& update_local_parameter(const void* data, size_t size, size_t offset) override;

        void push_debug_stage(const char* stage, const Color& color) override;
        void pop_debug_stage() override;

        template<typename Type>
        operator Type()
        {
            return Type();
        }
    };
}// namespace Engine
