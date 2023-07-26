#include <Core/logger.hpp>
#include <no_api.hpp>

namespace Engine
{
    NoApi& NoApi::logger(Logger*&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    void* NoApi::init_window(SDL_Window*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_window()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_object(Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_init()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_terminate()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_new_frame()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    ///////////////// TEXTURE PART /////////////////
    NoApi& NoApi::create_texture(Identifier&, const TextureCreateInfo&, TextureType type)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_texture(const Identifier&, TextureBindIndex)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MipMapLevel NoApi::base_level_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::base_level_texture(const Identifier&, MipMapLevel)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareFunc NoApi::compare_func_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::compare_func_texture(const Identifier&, CompareFunc)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareMode NoApi::compare_mode_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::compare_mode_texture(const Identifier&, CompareMode)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter NoApi::min_filter_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter NoApi::mag_filter_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::min_filter_texture(const Identifier&, TextureFilter)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::mag_filter_texture(const Identifier&, TextureFilter)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::min_lod_level_texture(const Identifier&, LodLevel)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::max_lod_level_texture(const Identifier&, LodLevel)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodLevel NoApi::min_lod_level_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodLevel NoApi::max_lod_level_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MipMapLevel NoApi::max_mipmap_level_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swizzle_texture(const Identifier&, const SwizzleRGBA&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    SwizzleRGBA NoApi::swizzle_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_s_texture(const Identifier&, const WrapValue&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_t_texture(const Identifier&, const WrapValue&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_r_texture(const Identifier&, const WrapValue&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_s_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_t_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_r_texture(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::anisotropic_filtering_texture(const Identifier& ID, float value)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    float NoApi::anisotropic_filtering_texture(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    float NoApi::max_anisotropic_filtering()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::texture_size(const Identifier&, Size2D&, MipMapLevel)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::generate_texture_mipmap(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    Identifier NoApi::imgui_texture_id(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    SamplerMipmapMode NoApi::sample_mipmap_mode_texture(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias NoApi::lod_bias_texture(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::lod_bias_texture(const Identifier& ID, LodBias bias)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias NoApi::max_lod_bias_texture()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel, const void*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&, const Offset2D&,
                                              MipMapLevel, void*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_vertex_buffer(Identifier&, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_vertex_buffer(const Identifier&, size_t offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_vertex_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_vertex_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_index_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::bind_index_buffer(const Identifier&, size_t offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    MappedMemory NoApi::map_index_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::unmap_index_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_uniform_buffer(Identifier&, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_uniform_buffer(const Identifier&, BindingIndex binding)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_uniform_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_uniform_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_framebuffer(const Identifier&, size_t buffer_index)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::framebuffer_viewport(const Identifier&, const ViewPort&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::framebuffer_scissor(const Identifier&, const Scissor&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_shader(Identifier&, const PipelineCreateInfo&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::use_shader(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_ssbo(Identifier&, const byte* data, size_t size)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_ssbo(const Identifier&, const byte*, size_t offset, size_t size)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swap_buffer(SDL_Window* window)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swap_interval(int_t interval)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::clear_color(const Identifier&, const ColorClearValue&, byte layout)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::clear_depth_stencil(const Identifier&, const DepthStencilClearValue&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::check_format_support(PixelType type, PixelComponentType component)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::on_window_size_changed()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::begin_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::end_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wait_idle()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::async_render(bool flag)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::async_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::next_render_thread()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    String NoApi::renderer()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return "Undefined";
    }
}// namespace Engine
