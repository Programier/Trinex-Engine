#pragma once

#include <Core/buffer_types.hpp>
#include <Core/color.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/render_types.hpp>
#include <Core/shader_types.hpp>
#include <Core/texture_types.hpp>

class SDL_Window;
namespace Engine
{
    class Logger;
}

#define VIRTUAL_METHOD

namespace Engine::GraphicApiInterface
{
    struct ENGINE_EXPORT ApiInterface {
        virtual ApiInterface& logger(Logger*&) VIRTUAL_METHOD;
        virtual void* init_window(SDL_Window*) VIRTUAL_METHOD;
        virtual ApiInterface& destroy_window() VIRTUAL_METHOD;
        virtual ApiInterface& destroy_object(Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& imgui_init() VIRTUAL_METHOD;
        virtual ApiInterface& imgui_terminate() VIRTUAL_METHOD;
        virtual ApiInterface& imgui_new_frame() VIRTUAL_METHOD;
        virtual ApiInterface& imgui_render() VIRTUAL_METHOD;

        ///////////////// TEXTURE PART /////////////////
        virtual ApiInterface& create_texture(Identifier&, const TextureCreateInfo&, TextureType type) VIRTUAL_METHOD;
        virtual ApiInterface& bind_texture(const Identifier&, TextureBindIndex) VIRTUAL_METHOD;

        virtual MipMapLevel base_level_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& base_level_texture(const Identifier&, MipMapLevel) VIRTUAL_METHOD;
        virtual CompareFunc compare_func_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& compare_func_texture(const Identifier&, CompareFunc) VIRTUAL_METHOD;
        virtual CompareMode compare_mode_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& compare_mode_texture(const Identifier&, CompareMode) VIRTUAL_METHOD;
        virtual TextureFilter min_filter_texture(const Identifier&) VIRTUAL_METHOD;
        virtual TextureFilter mag_filter_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& min_filter_texture(const Identifier&, TextureFilter) VIRTUAL_METHOD;
        virtual ApiInterface& mag_filter_texture(const Identifier&, TextureFilter) VIRTUAL_METHOD;
        virtual ApiInterface& min_lod_level_texture(const Identifier&, LodLevel) VIRTUAL_METHOD;
        virtual ApiInterface& max_lod_level_texture(const Identifier&, LodLevel) VIRTUAL_METHOD;
        virtual LodLevel min_lod_level_texture(const Identifier&) VIRTUAL_METHOD;
        virtual LodLevel max_lod_level_texture(const Identifier&) VIRTUAL_METHOD;
        virtual MipMapLevel max_mipmap_level_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& swizzle_texture(const Identifier&, const SwizzleRGBA&) VIRTUAL_METHOD;
        virtual SwizzleRGBA swizzle_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_s_texture(const Identifier&, const WrapValue&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_t_texture(const Identifier&, const WrapValue&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_r_texture(const Identifier&, const WrapValue&) VIRTUAL_METHOD;
        virtual WrapValue wrap_s_texture(const Identifier&) VIRTUAL_METHOD;
        virtual WrapValue wrap_t_texture(const Identifier&) VIRTUAL_METHOD;
        virtual WrapValue wrap_r_texture(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& anisotropic_filtering_texture(const Identifier& ID, float value) VIRTUAL_METHOD;
        virtual float anisotropic_filtering_texture(const Identifier& ID) VIRTUAL_METHOD;
        virtual float max_anisotropic_filtering() VIRTUAL_METHOD;
        virtual ApiInterface& texture_size(const Identifier&, Size2D&, MipMapLevel) VIRTUAL_METHOD;
        virtual ApiInterface& generate_texture_mipmap(const Identifier&) VIRTUAL_METHOD;
        virtual ApiInterface& read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel) VIRTUAL_METHOD;
        virtual Identifier imgui_texture_id(const Identifier&) VIRTUAL_METHOD;

        virtual SamplerMipmapMode sample_mipmap_mode_texture(const Identifier& ID) VIRTUAL_METHOD;
        virtual ApiInterface& sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode) VIRTUAL_METHOD;
        virtual LodBias lod_bias_texture(const Identifier& ID) VIRTUAL_METHOD;
        virtual ApiInterface& lod_bias_texture(const Identifier& ID, LodBias bias) VIRTUAL_METHOD;
        virtual LodBias max_lod_bias_texture() VIRTUAL_METHOD;

        virtual ApiInterface& update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel,
                                                const void*) VIRTUAL_METHOD;

        virtual ApiInterface& cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&,
                                                          const Offset2D&, MipMapLevel, void*) VIRTUAL_METHOD;

        virtual ApiInterface& create_vertex_buffer(Identifier&, const byte*, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& bind_vertex_buffer(const Identifier&, size_t offset) VIRTUAL_METHOD;
        virtual MappedMemory map_vertex_buffer(const Identifier& ID);
        virtual ApiInterface& unmap_vertex_buffer(const Identifier& ID);

        virtual ApiInterface& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) VIRTUAL_METHOD;
        virtual ApiInterface& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& bind_index_buffer(const Identifier&, size_t offset) VIRTUAL_METHOD;
        virtual MappedMemory map_index_buffer(const Identifier& ID);
        virtual ApiInterface& unmap_index_buffer(const Identifier& ID);

        virtual ApiInterface& create_uniform_buffer(Identifier&, const byte*, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& bind_uniform_buffer(const Identifier&, BindingIndex binding, size_t offset,
                                                  size_t size) VIRTUAL_METHOD;
        virtual MappedMemory map_uniform_buffer(const Identifier& ID);
        virtual ApiInterface& unmap_uniform_buffer(const Identifier& ID);

        virtual ApiInterface& draw_indexed(size_t indices_count, size_t indices_offset) VIRTUAL_METHOD;

        virtual ApiInterface& gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info) VIRTUAL_METHOD;
        virtual ApiInterface& bind_framebuffer(const Identifier&, size_t buffer_index = 0) VIRTUAL_METHOD;
        virtual ApiInterface& framebuffer_viewport(const Identifier&, const ViewPort&) VIRTUAL_METHOD;
        virtual ApiInterface& framebuffer_scissor(const Identifier&, const Scissor&) VIRTUAL_METHOD;

        virtual ApiInterface& create_shader(Identifier&, const PipelineCreateInfo&) VIRTUAL_METHOD;
        virtual ApiInterface& use_shader(const Identifier&) VIRTUAL_METHOD;

        virtual ApiInterface& create_ssbo(Identifier&, const byte* data, size_t size) VIRTUAL_METHOD;
        virtual ApiInterface& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) VIRTUAL_METHOD;
        virtual ApiInterface& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) VIRTUAL_METHOD;

        virtual ApiInterface& swap_buffer(SDL_Window* window) VIRTUAL_METHOD;
        virtual ApiInterface& swap_interval(int_t interval) VIRTUAL_METHOD;
        virtual ApiInterface& clear_color(const Identifier&, const ColorClearValue&, byte layout) VIRTUAL_METHOD;
        virtual ApiInterface& clear_depth_stencil(const Identifier&, const DepthStencilClearValue&) VIRTUAL_METHOD;

        virtual bool check_format_support(PixelType type, PixelComponentType component) VIRTUAL_METHOD;

        virtual ApiInterface& on_window_size_changed() VIRTUAL_METHOD;
        virtual ApiInterface& begin_render() VIRTUAL_METHOD;
        virtual ApiInterface& end_render() VIRTUAL_METHOD;
        virtual ApiInterface& wait_idle() VIRTUAL_METHOD;
        virtual ApiInterface& async_render(bool flag) VIRTUAL_METHOD;
        virtual bool async_render() VIRTUAL_METHOD;
        virtual ApiInterface& next_render_thread() VIRTUAL_METHOD;
        virtual String renderer() VIRTUAL_METHOD;

        virtual ~ApiInterface();
    };

#undef VIRTUAL_METHOD
}// namespace Engine::GraphicApiInterface
