#pragma once
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_state.hpp>


#define API (Engine::OpenGL::_M_open_gl)
#define opengl_debug_log debug_log
#define opengl_error error_log


namespace Engine
{
    struct OpenGL : public RHI {
        static OpenGL* _M_open_gl;
        struct WindowInterface* _M_window_interface = nullptr;

        void* _M_context                   = nullptr;
        byte _M_support_anisotropy : 1     = 0;
        ArrayOffset _M_index_buffer_offset = 0;
        size_t _M_current_buffer_index     = 0;
        size_t _M_next_buffer_index        = 1;
        Vector<BindingIndex> _M_samplers;

        OpenGL_State state;

        OpenGL();

        bool extension_supported(const String& extension_name);

        void* init_window(WindowInterface*, const WindowConfig&) override;
        OpenGL& destroy_window() override;
        OpenGL& destroy_object(Identifier&) override;
        OpenGL& imgui_init() override;
        OpenGL& imgui_terminate() override;
        OpenGL& imgui_new_frame() override;
        OpenGL& imgui_render() override;

        //        ///////////////// TEXTURE PART /////////////////
        OpenGL& internal_bind_texture(struct OpenGL_Texture* texture);
        Identifier imgui_texture_id(const Identifier&) override;

        OpenGL& create_vertex_buffer(Identifier&, const byte*, size_t) override;
        OpenGL& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        OpenGL& bind_vertex_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_vertex_buffer(const Identifier& ID) override;
        OpenGL& unmap_vertex_buffer(const Identifier& ID) override;

        OpenGL& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) override;
        OpenGL& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        OpenGL& bind_index_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_index_buffer(const Identifier& ID) override;
        OpenGL& unmap_index_buffer(const Identifier& ID) override;

        OpenGL& create_uniform_buffer(Identifier&, const byte*, size_t) override;
        OpenGL& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        OpenGL& bind_uniform_buffer(const Identifier&, BindingIndex binding) override;
        MappedMemory map_uniform_buffer(const Identifier& ID) override;
        OpenGL& unmap_uniform_buffer(const Identifier& ID) override;

        OpenGL& draw_indexed(size_t indices_count, size_t indices_offset) override;
        OpenGL& draw(size_t vertex_count) override;

        OpenGL& create_shader(Identifier&, const PipelineCreateInfo&) override;
        OpenGL& use_shader(const Identifier&) override;


        OpenGL& create_ssbo(Identifier&, const byte* data, size_t size) override;
        OpenGL& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        OpenGL& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        OpenGL& swap_buffer() override;
        OpenGL& vsync(bool) override;
        bool vsync() override;

        bool check_format_support(ColorFormat) override;

        OpenGL& on_window_size_changed() override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        OpenGL& async_render(bool flag) override;
        bool async_render() override;
        OpenGL& next_render_thread() override;
        String renderer() override;

        RHI_Sampler* create_sampler(const SamplerCreateInfo&) override;
        RHI_Texture* create_texture(const TextureCreateInfo&, TextureType type, const byte* data) override;
        RHI_FrameBuffer* window_framebuffer() override;
        RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;

        ~OpenGL();
    };
}// namespace Engine
