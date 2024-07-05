#pragma once

#include <Graphics/rhi.hpp>


namespace Engine
{
    class NoneApi : public RHI
    {
    public:
        static NoneApi* m_instance;

        NoneApi& initialize(class Window* window) override;
        void* context() override;
        NoneApi& destroy_object(RHI_Object* object) override;

        NoneApi& imgui_init(ImGuiContext*) override;
        NoneApi& imgui_terminate(ImGuiContext*) override;
        NoneApi& imgui_new_frame(ImGuiContext*) override;
        NoneApi& imgui_render(ImGuiContext*, ImDrawData*) override;
        NoneApi& draw(size_t vertex_count, size_t vertices_offset) override;
        NoneApi& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
        NoneApi& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances) override;
        NoneApi& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                        size_t instances) override;

        NoneApi& begin_render() override;
        NoneApi& end_render() override;
        NoneApi& wait_idle() override;

        void bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;
        void viewport(const ViewPort& viewport) override;
        ViewPort viewport() override;
        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture_2d(const Texture2D*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, RHIBufferType type) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
        RHI_Viewport* create_viewport(RenderViewport* viewport) override;

        NoneApi& push_global_params(const GlobalShaderParameters& params) override;
        NoneApi& pop_global_params() override;
        NoneApi& update_local_parameter(const void* data, size_t size, size_t offset) override;
        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;
    };
}// namespace Engine
