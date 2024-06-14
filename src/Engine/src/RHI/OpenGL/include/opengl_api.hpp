#pragma once
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_parameters.hpp>
#include <opengl_definitions.hpp>
#include <opengl_headers.hpp>

#define OPENGL_API Engine::OpenGL::m_instance

namespace Engine
{
    struct OpenGL : public RHI {
        static OpenGL* m_instance;
        void* m_context = nullptr;
        String m_renderer;

        // STATE
        struct OpenGL_Pipeline* m_current_pipeline        = nullptr;
        struct OpenGL_IndexBuffer* m_current_index_buffer = nullptr;
        Vector<BindingIndex> m_sampler_units;

        Vector<GlobalShaderParameters> m_global_parameters_stack;
        struct OpenGL_UniformBuffer* m_global_ubo     = nullptr;
        struct OpenGL_LocalUniformBuffer* m_local_ubo = nullptr;
        struct OpenGL_RenderTarget* m_render_target   = nullptr;
        ViewPort m_viewport;


        OpenGL();
        OpenGL& initialize_rhi();
        OpenGL& initialize_ubo();
        void* context() override;
        OpenGL& imgui_init(ImGuiContext*) override;
        OpenGL& imgui_terminate(ImGuiContext*) override;
        OpenGL& imgui_new_frame(ImGuiContext*) override;
        OpenGL& imgui_render(ImGuiContext*, ImDrawData*) override;

        OpenGL& prepare_render();
        OpenGL& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
        OpenGL& draw(size_t vertex_count, size_t vertices_offset) override;
        OpenGL& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
        OpenGL& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                       size_t instances) override;

        OpenGL& destroy_object(RHI_Object* object) override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        OpenGL& reset_state();
        const String& renderer() override;
        const String& name() override;

        void bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;
        void bind_render_target(const Span<struct OpenGL_Texture*>& color_attachments, struct OpenGL_Texture* depth_stencil);
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
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_Viewport* create_viewport(RenderViewport* viewport) override;

        OpenGL& push_global_params(const GlobalShaderParameters& params) override;
        OpenGL& pop_global_params() override;
        OpenGL& update_local_parameter(const void* data, size_t size, size_t offset) override;

        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;

        void reset_samplers();

        ~OpenGL();
    };
}// namespace Engine
