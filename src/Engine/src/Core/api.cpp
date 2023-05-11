#include <Core/logger.hpp>
#include <api.hpp>

namespace Engine::GraphicApiInterface
{
    ApiInterface& ApiInterface::logger(Logger*&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    void* ApiInterface::init_window(SDL_Window*)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    ApiInterface& ApiInterface::destroy_window()
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::destroy_object(Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::imgui_init()
    {
        return *this;
    }

    ApiInterface& ApiInterface::imgui_terminate()
    {
        return *this;
    }

    ApiInterface& ApiInterface::imgui_new_frame()
    {
        return *this;
    }

    ApiInterface& ApiInterface::imgui_render()
    {
        return *this;
    }

    ///////////////// TEXTURE PART /////////////////
    ApiInterface& ApiInterface::create_texture(Identifier&, const TextureCreateInfo&, TextureType)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_texture(const Identifier&, TextureBindIndex)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    MipMapLevel ApiInterface::base_level_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::base_level_texture(const Identifier&, MipMapLevel)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareFunc ApiInterface::compare_func_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return CompareFunc();
    }

    ApiInterface& ApiInterface::compare_func_texture(const Identifier&, CompareFunc)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareMode ApiInterface::compare_mode_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return CompareMode();
    }

    ApiInterface& ApiInterface::compare_mode_texture(const Identifier&, CompareMode)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter ApiInterface::min_filter_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return TextureFilter();
    }

    TextureFilter ApiInterface::mag_filter_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return TextureFilter();
    }

    ApiInterface& ApiInterface::min_filter_texture(const Identifier&, TextureFilter)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::mag_filter_texture(const Identifier&, TextureFilter)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::min_lod_level_texture(const Identifier&, LodLevel)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::max_lod_level_texture(const Identifier&, LodLevel)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodLevel ApiInterface::min_lod_level_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    LodLevel ApiInterface::max_lod_level_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    MipMapLevel ApiInterface::max_mipmap_level_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::swizzle_texture(const Identifier&, const SwizzleRGBA&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    SwizzleRGBA ApiInterface::swizzle_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return SwizzleRGBA();
    }

    ApiInterface& ApiInterface::wrap_s_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::wrap_t_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::wrap_r_texture(const Identifier&, const WrapValue&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue ApiInterface::wrap_s_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }

    WrapValue ApiInterface::wrap_t_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }

    WrapValue ApiInterface::wrap_r_texture(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }

    ApiInterface& ApiInterface::anisotropic_filtering_texture(const Identifier& ID, float value)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    float ApiInterface::anisotropic_filtering_texture(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0.0;
    }

    float ApiInterface::max_anisotropic_filtering()
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0.0;
    }

    ApiInterface& ApiInterface::update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel,
                                                  const void*)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::texture_size(const Identifier&, Size2D&, MipMapLevel)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::generate_texture_mipmap(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::ApiInterface::read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    Identifier ApiInterface::imgui_texture_id(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    SamplerMipmapMode ApiInterface::sample_mipmap_mode_texture(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return SamplerMipmapMode();
    }

    ApiInterface& ApiInterface::sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias ApiInterface::lod_bias_texture(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0.0;
    }

    ApiInterface& ApiInterface::lod_bias_texture(const Identifier& ID, LodBias bias)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    LodBias ApiInterface::max_lod_bias_texture()
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0.0;
    }

    ApiInterface& ApiInterface::cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&,
                                                            const Offset2D&, MipMapLevel, void*)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::create_vertex_buffer(Identifier&, const byte*, size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_vertex_buffer(const Identifier&, size_t offset)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory ApiInterface::map_vertex_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return MappedMemory(nullptr, 0);
    }

    ApiInterface& ApiInterface::unmap_vertex_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::update_index_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_index_buffer(const Identifier&, size_t offset)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory ApiInterface::map_index_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return MappedMemory(nullptr, 0);
    }

    ApiInterface& ApiInterface::unmap_index_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }


    ApiInterface& ApiInterface::create_uniform_buffer(Identifier&, const byte*, size_t)
    {
        return *this;
    }

    ApiInterface& ApiInterface::update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        return *this;
    }

    ApiInterface& ApiInterface::bind_uniform_buffer(const Identifier&, BindingIndex, size_t offset, size_t size)
    {
        return *this;
    }

    MappedMemory ApiInterface::map_uniform_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return MappedMemory(nullptr, 0);
    }

    ApiInterface& ApiInterface::unmap_uniform_buffer(const Identifier& ID)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }


    ApiInterface& ApiInterface::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_framebuffer(const Identifier&, size_t buffer_index)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::framebuffer_viewport(const Identifier&, const ViewPort&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }


    ApiInterface& ApiInterface::create_shader(Identifier&, const PipelineCreateInfo&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::use_shader(const Identifier&)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }


    ApiInterface& ApiInterface::create_ssbo(Identifier&, const byte*, size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_ssbo(const Identifier&, BindingIndex slot, size_t, size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }


    ApiInterface& ApiInterface::update_ssbo(const Identifier&, const byte*, std::size_t, std::size_t)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::swap_buffer(SDL_Window* window)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::swap_interval(int_t interval)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::clear_color(const Identifier&, const ColorClearValue&, byte layout)
    {
        Engine::logger->warning("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::on_window_size_changed()
    {
        return *this;
    }

    ApiInterface& ApiInterface::begin_render()
    {
        return *this;
    }

    ApiInterface& ApiInterface::end_render()
    {
        return *this;
    }

    ApiInterface& ApiInterface::wait_idle()
    {
        return *this;
    }

    ApiInterface& ApiInterface::async_render(bool flag)
    {
        return *this;
    }

    bool ApiInterface::async_render()
    {
        return false;
    }

    ApiInterface& ApiInterface::next_render_thread()
    {
        return *this;
    }

    String ApiInterface::renderer()
    {
        return "Undefined";
    }

    bool ApiInterface::check_format_support(PixelType type, PixelComponentType component)
    {
        return false;
    }

    ApiInterface& ApiInterface::clear_depth_stencil(const Identifier&, const DepthStencilClearValue&)
    {
        return *this;
    }

    ApiInterface& ApiInterface::framebuffer_scissor(const Identifier&, const Scissor&)
    {
        return *this;
    }

    ApiInterface::~ApiInterface()
    {}
}// namespace Engine::GraphicApiInterface
