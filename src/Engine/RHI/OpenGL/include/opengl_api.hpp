#pragma once
#include <Core/logger.hpp>
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


        WindowInterface* _M_window = nullptr;
        void* _M_surface           = nullptr;

        struct OpenGL_RenderTarget* _M_main_render_target = nullptr;

        // STATE
        struct OpenGL_RenderTarget* _M_current_render_target = nullptr;
        struct OpenGL_Pipeline* _M_current_pipeline          = nullptr;
        struct OpenGL_IndexBuffer* _M_current_index_buffer   = nullptr;

        Vector<BindingIndex> _M_sampler_units;


        OpenGL();

        void* init_window(struct WindowInterface*, const WindowConfig& config) override;
        OpenGL& destroy_window() override;
        OpenGL& imgui_init() override;
        OpenGL& imgui_terminate() override;
        OpenGL& imgui_new_frame() override;
        OpenGL& imgui_render() override;

        //        ///////////////// TEXTURE PART /////////////////
        //        Identifier imgui_texture_id(const Identifier&) override;

        OpenGL& draw_indexed(size_t indices_count, size_t indices_offset) override;
        OpenGL& draw(size_t vertex_count) override;
        OpenGL& swap_buffer() override;
        //        RHI& vsync(bool) override;
        //        bool vsync() override;
        //        bool check_format_support(ColorFormat) override;

        //        RHI& on_window_size_changed() override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        String renderer() override;

        //        // Bariers
        //        RHI& push_barrier(Texture* texture, BarrierStage src, BarrierStage dst) override;


        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data) override;
        RHI_RenderTarget* window_render_target() override;
        RHI_RenderTarget* create_render_target(const RenderTarget* render_target) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data) override;
        //        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass() override;
        ColorFormatFeatures color_format_features(ColorFormat format) override;

        void push_debug_stage(const char* stage, const Color& color = {}) override;
        void pop_debug_stage() override;

        void reset_samplers();
    };
}// namespace Engine
