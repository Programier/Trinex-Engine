#pragma once
#include <api.hpp>

namespace Engine
{
    class NoApi : public GraphicApiInterface::ApiInterface
    {
        NoApi& logger(Logger*&) override;
        void* init_window(SDL_Window*) override;
        NoApi& destroy_window() override;
        NoApi& destroy_object(Identifier&) override;
        NoApi& imgui_init() override;
        NoApi& imgui_terminate() override;
        NoApi& imgui_new_frame() override;
        NoApi& imgui_render() override;

        ///////////////// TEXTURE PART /////////////////
        NoApi& create_texture(Identifier&, const TextureCreateInfo&, TextureType type) override;
        NoApi& bind_texture(const Identifier&, TextureBindIndex) override;

        MipMapLevel base_level_texture(const Identifier&) override;
        NoApi& base_level_texture(const Identifier&, MipMapLevel) override;
        CompareFunc compare_func_texture(const Identifier&) override;
        NoApi& compare_func_texture(const Identifier&, CompareFunc) override;
        CompareMode compare_mode_texture(const Identifier&) override;
        NoApi& compare_mode_texture(const Identifier&, CompareMode) override;
        TextureFilter min_filter_texture(const Identifier&) override;
        TextureFilter mag_filter_texture(const Identifier&) override;
        NoApi& min_filter_texture(const Identifier&, TextureFilter) override;
        NoApi& mag_filter_texture(const Identifier&, TextureFilter) override;
        NoApi& min_lod_level_texture(const Identifier&, LodLevel) override;
        NoApi& max_lod_level_texture(const Identifier&, LodLevel) override;
        LodLevel min_lod_level_texture(const Identifier&) override;
        LodLevel max_lod_level_texture(const Identifier&) override;
        MipMapLevel max_mipmap_level_texture(const Identifier&) override;
        NoApi& swizzle_texture(const Identifier&, const SwizzleRGBA&) override;
        SwizzleRGBA swizzle_texture(const Identifier&) override;
        NoApi& wrap_s_texture(const Identifier&, const WrapValue&) override;
        NoApi& wrap_t_texture(const Identifier&, const WrapValue&) override;
        NoApi& wrap_r_texture(const Identifier&, const WrapValue&) override;
        WrapValue wrap_s_texture(const Identifier&) override;
        WrapValue wrap_t_texture(const Identifier&) override;
        WrapValue wrap_r_texture(const Identifier&) override;
        NoApi& anisotropic_filtering_texture(const Identifier& ID, float value) override;
        float anisotropic_filtering_texture(const Identifier& ID) override;
        float max_anisotropic_filtering() override;
        NoApi& texture_size(const Identifier&, Size2D&, MipMapLevel) override;
        NoApi& generate_texture_mipmap(const Identifier&) override;
        NoApi& read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel) override;
        Identifier imgui_texture_id(const Identifier&) override;

        SamplerMipmapMode sample_mipmap_mode_texture(const Identifier& ID) override;
        NoApi& sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode) override;
        LodBias lod_bias_texture(const Identifier& ID) override;
        NoApi& lod_bias_texture(const Identifier& ID, LodBias bias) override;
        LodBias max_lod_bias_texture() override;

        NoApi& update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel, const void*) override;

        NoApi& cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&, const Offset2D&,
                                           MipMapLevel, void*) override;

        NoApi& create_vertex_buffer(Identifier&, const byte*, size_t) override;
        NoApi& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_vertex_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_vertex_buffer(const Identifier& ID) override;
        NoApi& unmap_vertex_buffer(const Identifier& ID) override;

        NoApi& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) override;
        NoApi& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_index_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_index_buffer(const Identifier& ID) override;
        NoApi& unmap_index_buffer(const Identifier& ID) override;

        NoApi& create_uniform_buffer(Identifier&, const byte*, size_t) override;
        NoApi& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_uniform_buffer(const Identifier&, BindingIndex binding) override;
        MappedMemory map_uniform_buffer(const Identifier& ID) override;
        NoApi& unmap_uniform_buffer(const Identifier& ID) override;

        NoApi& draw_indexed(size_t indices_count, size_t indices_offset) override;

        NoApi& gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info) override;
        NoApi& bind_framebuffer(const Identifier&, size_t buffer_index = 0) override;
        NoApi& framebuffer_viewport(const Identifier&, const ViewPort&) override;
        NoApi& framebuffer_scissor(const Identifier&, const Scissor&) override;

        NoApi& create_shader(Identifier&, const PipelineCreateInfo&) override;
        NoApi& use_shader(const Identifier&) override;

        NoApi& create_ssbo(Identifier&, const byte* data, size_t size) override;
        NoApi& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        NoApi& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        NoApi& swap_buffer(SDL_Window* window) override;
        NoApi& swap_interval(int_t interval) override;
        NoApi& clear_color(const Identifier&, const ColorClearValue&, byte layout) override;
        NoApi& clear_depth_stencil(const Identifier&, const DepthStencilClearValue&) override;

        bool check_format_support(PixelType type, PixelComponentType component) override;

        NoApi& on_window_size_changed() override;
        NoApi& begin_render() override;
        NoApi& end_render() override;
        NoApi& wait_idle() override;
        NoApi& async_render(bool flag) override;
        bool async_render() override;
        NoApi& next_render_thread() override;
        String renderer() override;


        template<typename Type>
        operator Type()
        {
            return Type();
        }
    };
}// namespace Engine
