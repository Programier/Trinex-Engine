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
        struct OpenGL_RenderPass* m_main_render_pass = nullptr;

        // STATE
        struct OpenGL_RenderTarget* m_current_render_target = nullptr;
        struct OpenGL_RenderPass* m_current_render_pass     = nullptr;
        struct OpenGL_Pipeline* m_current_pipeline          = nullptr;
        struct OpenGL_IndexBuffer* m_current_index_buffer   = nullptr;
        Vector<BindingIndex> m_sampler_units;

        Vector<GlobalShaderParameters> m_global_parameters_stack;
        struct OpenGL_UniformBuffer* m_global_ubo     = nullptr;
        struct OpenGL_LocalUniformBuffer* m_local_ubo = nullptr;


        OpenGL();
        OpenGL& initialize();
        OpenGL& initialize_ubo();
        ~OpenGL();
        OpenGL& imgui_init(ImGuiContext*) override;
        OpenGL& imgui_terminate(ImGuiContext*) override;
        OpenGL& imgui_new_frame(ImGuiContext*) override;
        OpenGL& imgui_render(ImGuiContext*, ImDrawData*) override;

        OpenGL& prepare_render();
        OpenGL& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) override;
        OpenGL& draw(size_t vertex_count, size_t vertices_offset) override;
        OpenGL& draw_instanced(size_t vertex_count, size_t instances) override;
        OpenGL& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t instances) override;

        OpenGL& destroy_object(RHI_Object* object) override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        OpenGL& reset_state();
        const String& renderer() override;
        const String& name() override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data, size_t size) override;
        RHI_RenderTarget* create_render_target(const RenderTarget* render_target) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass(RenderPass* engine_render_pass) override;
        ColorFormatFeatures color_format_features(ColorFormat format) override;

        RHI_Viewport* create_viewport(WindowInterface* interface, bool vsync) override;
        RHI_Viewport* create_viewport(RenderTarget* render_target) override;

        OpenGL& push_global_params(const GlobalShaderParameters& params) override;
        OpenGL& pop_global_params() override;
        OpenGL& update_local_parameter(const void* data, size_t size, size_t offset) override;

        ColorFormat base_color_format() override;
        ColorFormat position_format() override;
        ColorFormat normal_format() override;
        ColorFormat emissive_format() override;
        ColorFormat msra_buffer_format() override;
        ColorFormat depth_format() override;
        ColorFormat stencil_format() override;
        ColorFormat depth_stencil_format() override;

        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;

        void reset_samplers();
    };
}// namespace Engine
