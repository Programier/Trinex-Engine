#include <Core/logger.hpp>
#include <no_api.hpp>

namespace Engine
{
    NoApi& NoApi::logger(Logger*&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    void* NoApi::init_window(SDL_Window*)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_window()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_object(Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_init()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_terminate()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_new_frame()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_render()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    ///////////////// TEXTURE PART /////////////////
    NoApi& NoApi::create_texture(Identifier&, const TextureCreateInfo&, TextureType type)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_texture(const Identifier&, TextureBindIndex)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MipMapLevel NoApi::base_level_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::base_level_texture(const Identifier&, MipMapLevel)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareFunc NoApi::compare_func_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::compare_func_texture(const Identifier&, CompareFunc)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareMode NoApi::compare_mode_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::compare_mode_texture(const Identifier&, CompareMode)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter NoApi::min_filter_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter NoApi::mag_filter_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::min_filter_texture(const Identifier&, TextureFilter)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::mag_filter_texture(const Identifier&, TextureFilter)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::min_lod_level_texture(const Identifier&, LodLevel)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::max_lod_level_texture(const Identifier&, LodLevel)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodLevel NoApi::min_lod_level_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodLevel NoApi::max_lod_level_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MipMapLevel NoApi::max_mipmap_level_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swizzle_texture(const Identifier&, const SwizzleRGBA&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    SwizzleRGBA NoApi::swizzle_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_s_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_t_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wrap_r_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_s_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_t_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue NoApi::wrap_r_texture(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::anisotropic_filtering_texture(const Identifier& ID, float value)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    float NoApi::anisotropic_filtering_texture(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    float NoApi::max_anisotropic_filtering()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::texture_size(const Identifier&, Size2D&, MipMapLevel)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::generate_texture_mipmap(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    Identifier NoApi::imgui_texture_id(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    SamplerMipmapMode NoApi::sample_mipmap_mode_texture(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias NoApi::lod_bias_texture(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::lod_bias_texture(const Identifier& ID, LodBias bias)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias NoApi::max_lod_bias_texture()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel, const void*)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&, const Offset2D&,
                                              MipMapLevel, void*)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_vertex_buffer(Identifier&, const byte*, size_t)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_vertex_buffer(const Identifier&, size_t offset)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_vertex_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_vertex_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_index_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::bind_index_buffer(const Identifier&, size_t offset)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    MappedMemory NoApi::map_index_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::unmap_index_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_uniform_buffer(Identifier&, const byte*, size_t)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_uniform_buffer(const Identifier&, BindingIndex binding)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_uniform_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_uniform_buffer(const Identifier& ID)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_framebuffer(const Identifier&, size_t buffer_index)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::framebuffer_viewport(const Identifier&, const ViewPort&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::framebuffer_scissor(const Identifier&, const Scissor&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_shader(Identifier&, const PipelineCreateInfo&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::use_shader(const Identifier&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_ssbo(Identifier&, const byte* data, size_t size)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_ssbo(const Identifier&, const byte*, size_t offset, size_t size)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swap_buffer(SDL_Window* window)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swap_interval(int_t interval)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::clear_color(const Identifier&, const ColorClearValue&, byte layout)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::clear_depth_stencil(const Identifier&, const DepthStencilClearValue&)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::check_format_support(PixelType type, PixelComponentType component)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::on_window_size_changed()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::begin_render()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::end_render()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wait_idle()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::async_render(bool flag)
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::async_render()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::next_render_thread()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    String NoApi::renderer()
    {
        Engine::logger->error("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return "Undefined";
    }
}// namespace Engine
