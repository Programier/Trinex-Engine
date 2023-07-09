#pragma once
#include <Core/logger.hpp>
#include <api.hpp>


class SDL_Window;

#define API (Engine::OpenGL::_M_open_gl)
#define opengl_debug_log (*OpenGL::_M_open_gl->_M_logger)->log
#define opengl_error (*OpenGL::_M_open_gl->_M_logger)->error


namespace Engine
{
    struct OpenGL : public GraphicApiInterface::ApiInterface {
        static OpenGL* _M_open_gl;

        Logger** _M_logger                                   = nullptr;
        void* _M_context                                     = nullptr;
        byte _M_support_anisotropy : 1                       = 0;
        struct OpenGL_Shader* _M_current_shader              = nullptr;
        ArrayOffset _M_index_buffer_offset                   = 0;
        struct OpenGL_IndexBuffer* _M_index_buffer           = nullptr;
        struct OpenGL_FrameBufferSet* _M_current_framebuffer = nullptr;
        size_t _M_current_buffer_index                       = 0;
        size_t _M_next_buffer_index                          = 1;

        OpenGL();
        struct OpenGL_FrameBufferSet* framebuffer(Identifier ID);


        bool extension_supported(const String& extension_name);

        OpenGL& logger(Logger*&) override;
        void* init_window(SDL_Window*) override;
        OpenGL& destroy_window() override;
        OpenGL& destroy_object(Identifier&) override;
        OpenGL& imgui_init() override;
        OpenGL& imgui_terminate() override;
        OpenGL& imgui_new_frame() override;
        OpenGL& imgui_render() override;

        //        ///////////////// TEXTURE PART /////////////////
        OpenGL& create_texture(Identifier&, const TextureCreateInfo&, TextureType type) override;
        OpenGL& internal_bind_texture(struct OpenGL_Texture* texture);
        OpenGL& bind_texture(const Identifier&, TextureBindIndex) override;

        MipMapLevel base_level_texture(const Identifier&) override;
        OpenGL& base_level_texture(const Identifier&, MipMapLevel) override;
        CompareFunc compare_func_texture(const Identifier&) override;
        OpenGL& compare_func_texture(const Identifier&, CompareFunc) override;
        CompareMode compare_mode_texture(const Identifier&) override;
        OpenGL& compare_mode_texture(const Identifier&, CompareMode) override;
        TextureFilter min_filter_texture(const Identifier&) override;
        TextureFilter mag_filter_texture(const Identifier&) override;
        OpenGL& min_filter_texture(const Identifier&, TextureFilter) override;
        OpenGL& mag_filter_texture(const Identifier&, TextureFilter) override;
        OpenGL& min_lod_level_texture(const Identifier&, LodLevel) override;
        OpenGL& max_lod_level_texture(const Identifier&, LodLevel) override;
        LodLevel min_lod_level_texture(const Identifier&) override;
        LodLevel max_lod_level_texture(const Identifier&) override;
        MipMapLevel max_mipmap_level_texture(const Identifier&) override;
        OpenGL& swizzle_texture(const Identifier&, const SwizzleRGBA&) override;
        SwizzleRGBA swizzle_texture(const Identifier&) override;
        OpenGL& wrap_s_texture(const Identifier&, const WrapValue&) override;
        OpenGL& wrap_t_texture(const Identifier&, const WrapValue&) override;
        OpenGL& wrap_r_texture(const Identifier&, const WrapValue&) override;
        WrapValue wrap_s_texture(const Identifier&) override;
        WrapValue wrap_t_texture(const Identifier&) override;
        WrapValue wrap_r_texture(const Identifier&) override;
        OpenGL& anisotropic_filtering_texture(const Identifier& ID, float value) override;
        float anisotropic_filtering_texture(const Identifier& ID) override;
        float max_anisotropic_filtering() override;
        OpenGL& texture_size(const Identifier&, Size2D&, MipMapLevel) override;
        OpenGL& generate_texture_mipmap(const Identifier&) override;
        OpenGL& read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel) override;
        Identifier imgui_texture_id(const Identifier&) override;
        SamplerMipmapMode sample_mipmap_mode_texture(const Identifier& ID) override;
        OpenGL& sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode) override;
        LodBias lod_bias_texture(const Identifier& ID) override;
        OpenGL& lod_bias_texture(const Identifier& ID, LodBias bias) override;
        LodBias max_lod_bias_texture() override;
        OpenGL& update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel, const void*) override;

        OpenGL& cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&, const Offset2D&,
                                            MipMapLevel, void*) override;

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

        OpenGL& gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info) override;
        OpenGL& bind_framebuffer(const Identifier&, size_t buffer_index) override;
        OpenGL& framebuffer_viewport(const Identifier&, const ViewPort&) override;
        OpenGL& framebuffer_scissor(const Identifier&, const Scissor&) override;

        OpenGL& create_shader(Identifier&, const PipelineCreateInfo&) override;
        OpenGL& use_shader(const Identifier&) override;


        //        OpenGL& create_ssbo(Identifier&, const byte* data, size_t size) override;
        //        OpenGL& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        //        OpenGL& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        OpenGL& swap_buffer(SDL_Window* window) override;
        OpenGL& swap_interval(int_t interval) override;
        //        OpenGL& clear_color(const Identifier&, const ColorClearValue&, byte layout) override;
        //        OpenGL& clear_depth_stencil(const Identifier&, const DepthStencilClearValue&) override;

        //        bool check_format_support(PixelType type, PixelComponentType component) override;

        OpenGL& on_window_size_changed() override;
        OpenGL& begin_render() override;
        OpenGL& end_render() override;
        OpenGL& wait_idle() override;
        String renderer() override;

        ~OpenGL();
    };
}// namespace Engine
