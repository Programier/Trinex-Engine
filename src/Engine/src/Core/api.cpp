#include <Core/logger.hpp>
#include <api.hpp>

namespace Engine::GraphicApiInterface
{

#ifdef ENGINE_API_IMPLEMENTATION
    ApiInterface& ApiInterface::logger(Logger*&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    void* ApiInterface::init_window(SDL_Window*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    ApiInterface& ApiInterface::destroy_window()
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::destroy_object(ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ///////////////// TEXTURE PART /////////////////
    ApiInterface& ApiInterface::create_texture(ObjID&, const TextureParams&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_texture(const ObjID&, TextureBindIndex)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    const TextureParams* ApiInterface::param_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    int_t ApiInterface::base_level_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::base_level_texture(const ObjID&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::depth_stencil_mode_texture(const ObjID&, DepthStencilMode)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    DepthStencilMode ApiInterface::depth_stencil_mode_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return DepthStencilMode();
    }

    CompareFunc ApiInterface::compare_func_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return CompareFunc();
    }

    ApiInterface& ApiInterface::compare_func_texture(const ObjID&, CompareFunc)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    CompareMode ApiInterface::compare_mode_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return CompareMode();
    }

    ApiInterface& ApiInterface::compare_mode_texture(const ObjID&, CompareMode)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    TextureFilter ApiInterface::min_filter_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return TextureFilter();
    }

    TextureFilter ApiInterface::mag_filter_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return TextureFilter();
    }

    ApiInterface& ApiInterface::min_filter_texture(const ObjID&, TextureFilter)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::mag_filter_texture(const ObjID&, TextureFilter)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::min_lod_level_texture(const ObjID&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::max_lod_level_texture(const ObjID&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::max_mipmap_level_texture(const ObjID&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    int_t ApiInterface::min_lod_level_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    int_t ApiInterface::max_lod_level_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    int_t ApiInterface::max_mipmap_level_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::swizzle_texture(const ObjID&, const SwizzleRGBA&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    SwizzleRGBA ApiInterface::swizzle_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return SwizzleRGBA();
    }

    ApiInterface& ApiInterface::wrap_s_texture(const ObjID&, const WrapValue&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::wrap_t_texture(const ObjID&, const WrapValue&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::wrap_r_texture(const ObjID&, const WrapValue&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    WrapValue ApiInterface::wrap_s_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }

    WrapValue ApiInterface::wrap_t_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }

    WrapValue ApiInterface::wrap_r_texture(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return WrapValue();
    }
    ApiInterface& ApiInterface::copy_read_buffer_to_texture_2D(const ObjID&, const Size2D&, const Point2D&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::texture_2D_update_from_current_read_buffer(const ObjID&, const Size2D&, const Offset2D&,
                                                                           const Point2D&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::gen_texture_2D(const ObjID&, const Size2D&, int_t, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::update_texture_2D(const ObjID&, const Size2D&, const Offset2D&, int_t, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::texture_size(const ObjID&, Size3D&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::generate_texture_mipmap(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::ApiInterface::read_texture_2D_data(const ObjID&, std::vector<byte>& data, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ObjID ApiInterface::texture_id(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::cubemap_texture_attach_2d_texture(const ObjID&, const ObjID&, TextureCubeMapFace, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::cubemap_texture_attach_data(const ObjID&, TextureCubeMapFace, const Size2D&, int_t,
                                                            void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::generate_mesh(ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::mesh_data(const ObjID&, std::size_t, DrawMode, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::draw_mesh(const ObjID&, Primitive, std::size_t, std::size_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::update_mesh_data(const ObjID&, std::size_t, std::size_t, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::mesh_indexes_array(const ObjID&, std::size_t, const IndexBufferComponent&, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::update_mesh_indexes_array(const ObjID&, size_t, size_t, void* data)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::gen_framebuffer(ObjID&, FrameBufferType)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::clear_frame_buffer(const ObjID&, BufferType)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_framebuffer(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::framebuffer_viewport(const Point2D&, const Size2D&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }
    ApiInterface& ApiInterface::attach_texture_to_framebuffer(const ObjID&, const ObjID&, FrameBufferAttach,
                                                              TextureAttachIndex, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::enable(Engine::EnableCap)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::disable(Engine::EnableCap)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::blend_func(Engine::BlendFunc, Engine::BlendFunc)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::depth_func(Engine::DepthFunc)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    float ApiInterface::line_rendering_width()
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return 0;
    }

    ApiInterface& ApiInterface::line_rendering_width(float)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::create_shader(ObjID&, const ShaderParams&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::use_shader(const ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, float)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, int_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, const glm::mat3&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, const glm::mat4&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, const glm::vec2&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, const glm::vec3&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const std::string&, const glm::vec4&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::shader_value(const ObjID&, const String&, void*)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::depth_mask(bool)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::stencil_mask(byte)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::stencil_func(Engine::CompareFunc, int_t, byte)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::stencil_option(Engine::StencilOption, Engine::StencilOption, Engine::StencilOption)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::create_ssbo(ObjID&)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::bind_ssbo(const ObjID&, int_t slot)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::ssbo_data(const ObjID&, void*, std::size_t, BufferUsage)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::update_ssbo_data(const ObjID&, void*, std::size_t, std::size_t)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::swap_buffer(SDL_Window* window)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::swap_interval(int_t interval)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
        return *this;
    }

    ApiInterface& ApiInterface::clear_color(const Color& color)
    {
        Engine::logger->log("METHOD %s IS NOT IMPLEMENTED!", __PRETTY_FUNCTION__);
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

    ApiInterface& ApiInterface::begin_render_pass()
    {
        return *this;
    }

    ApiInterface& ApiInterface::end_render_pass()
    {
        return *this;
    }

    ApiInterface& ApiInterface::wait_idle()
    {
        return *this;
    }

#endif
    ApiInterface::~ApiInterface()
    {}
}// namespace Engine::GraphicApiInterface
