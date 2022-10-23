#include <Core/implement.hpp>
#include <Graphics/texture.hpp>
#include <api_funcs.hpp>

using namespace Engine;

implement_class_cpp(Texture);

Texture::Texture(const TextureParams& params)
{
    create(params);
}

Texture& Texture::create(const TextureParams& params)
{
    Object::destroy();
    create_texture(_M_ID, params);
    return *this;
}

const Texture& Texture::bind(unsigned int num) const
{
    bind_texture(_M_ID, num);
    return *this;
}

int Texture::base_level() const
{
    return get_base_level_texture(_M_ID);
}

Texture& Texture::base_level(int level)
{
    set_base_level_texture(_M_ID, level);
    return *this;
}

Texture& Texture::depth_stencil_mode(DepthStencilMode mode)
{
    set_depth_stencil_mode_texture(_M_ID, mode);
    return *this;
}

DepthStencilMode Texture::depth_stencil_mode() const
{
    return get_depth_stencil_mode_texture(_M_ID);
}

CompareFunc Texture::compare_func() const
{
    return get_compare_func_texture(_M_ID);
}

Texture& Texture::compare_func(CompareFunc func)
{
    set_compare_func_texture(_M_ID, func);
    return *this;
}

Texture& Texture::compare_mode(CompareMode mode)
{
    set_compare_mode_texture(_M_ID, mode);
    return *this;
}

CompareMode Texture::compare_mode() const
{
    return get_compare_mode_texture(_M_ID);
}

const TextureFilter Texture::min_filter() const
{
    return get_min_filter_texture(_M_ID);
}

const TextureFilter Texture::mag_filter() const
{
    return get_mag_filter_texture(_M_ID);
}

Texture& Texture::min_filter(TextureFilter filter)
{
    set_min_filter_texture(_M_ID, filter);
    return *this;
}

Texture& Texture::mag_filter(TextureFilter filter)
{
    set_mag_filter_texture(_M_ID, filter);
    return *this;
}

Texture& Texture::min_lod_level(int value)
{
    set_min_lod_level_texture(_M_ID, value);
    return *this;
}

Texture& Texture::max_lod_level(int value)
{
    set_max_lod_level_texture(_M_ID, value);
    return *this;
}

Texture& Texture::max_mipmap_level(int value)
{
    set_max_mipmap_level_texture(_M_ID, value);
    return *this;
}

int Texture::min_lod_level() const
{
    return get_min_lod_level_texture(_M_ID);
}

int Texture::max_lod_level() const
{
    return get_max_lod_level_texture(_M_ID);
}

int Texture::max_mipmap_level() const
{
    return get_max_mipmap_level_texture(_M_ID);
}

SwizzleRGBA Texture::swizzle() const
{
    return get_swizzle_texture(_M_ID);
}

Texture& Texture::swizzle(const SwizzleRGBA& value)
{
    set_swizzle_texture(_M_ID, value);
    return *this;
}

Texture& Texture::wrap_s(WrapValue& wrap)
{
    set_wrap_s_texture(_M_ID, wrap);
    return *this;
}

Texture& Texture::wrap_t(WrapValue& wrap)
{
    set_wrap_t_texture(_M_ID, wrap);
    return *this;
}

Texture& Texture::wrap_r(WrapValue& wrap)
{
    set_wrap_r_texture(_M_ID, wrap);
    return *this;
}

WrapValue Texture::wrap_s() const
{
    return get_wrap_s_texture(_M_ID);
}

WrapValue Texture::wrap_t() const
{
    return get_wrap_t_texture(_M_ID);
}

WrapValue Texture::wrap_r() const
{
    return get_wrap_r_texture(_M_ID);
}

Texture& Texture::generate_mipmap()
{
    generate_texture_mipmap(_M_ID);
    return *this;
}

Size3D Texture::size(int level) const
{
    Size3D _M_size;
    get_size_texture(_M_ID, _M_size, level);
    return _M_size;
}

ObjID Texture::internal_id() const
{
    return texture_id(_M_ID);
}
