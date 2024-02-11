#pragma once
#include <Core/logger.hpp>
#include <Graphics/global_shader_parameters.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_definitions.hpp>
#include <opengl_headers.hpp>

#if OPENGL_EXTENDS_FROM_NOAPI
#include <no_api.hpp>
#endif

#define OPENGL_API Engine::OpenGL::_M_instance

namespace Engine
{
    struct OpenGL :
#if OPENGL_EXTENDS_FROM_NOAPI
        public NoApi
#else
        public RHI
#endif
    {

        static OpenGL* _M_instance;
        void* _M_context = nullptr;
        String _M_renderer;
        struct OpenGL_RenderPass* _M_main_render_pass = nullptr;

        // STATE
        struct OpenGL_RenderTarget* _M_current_render_target = nullptr;
        struct OpenGL_Pipeline* _M_current_pipeline          = nullptr;
        struct OpenGL_IndexBuffer* _M_current_index_buffer   = nullptr;
        Vector<BindingIndex> _M_sampler_units;

        Vector<GlobalShaderParameters> _M_global_parameters_stack;
        struct OpenGL_UniformBuffer* _M_global_ubo = nullptr;
        struct OpenGL_UniformBuffer* _M_local_ubo  = nullptr;


        OpenGL();
        OpenGL& initialize();
        OpenGL& initialize_ubo();
        ~OpenGL();
        OpenGL& imgui_init(ImGuiContext*) override;
        OpenGL& imgui_terminate(ImGuiContext*) override;
        OpenGL& imgui_new_frame(ImGuiContext*) override;
        OpenGL& imgui_render(ImGuiContext*, ImDrawData*) override;
        RHI_ImGuiTexture* imgui_create_texture(ImGuiContext*, Texture* texture, Sampler* sampler) override;

        //        ///////////////// TEXTURE PART /////////////////
        OpenGL& prepare_render();
        OpenGL& draw_indexed(size_t indices_count, size_t indices_offset) override;
        OpenGL& draw(size_t vertex_count) override;

        OpenGL& destroy_object(RHI_Object* object) override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        OpenGL& reset_state();
        const String& renderer() override;
        const String& name() override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data) override;
        RHI_RenderTarget* create_render_target(const RenderTarget* render_target) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass() override;
        ColorFormatFeatures color_format_features(ColorFormat format) override;
        size_t render_target_buffer_count() override;
        void line_width(float width) override;

        RHI_Viewport* create_viewport(WindowInterface* interface, bool vsync) override;
        RHI_Viewport* create_viewport(RenderTarget* render_target) override;

        OpenGL& push_global_params(GlobalShaderParameters* params = nullptr) override;
        OpenGL& update_global_params(void* data, size_t size, size_t offset) override;
        OpenGL& pop_global_params() override;

        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;

        void reset_samplers();
    };
}// namespace Engine
