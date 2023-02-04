#include <Core/engine.hpp>
#include <Core/implement.hpp>
#include <Graphics/texture.hpp>
#include <api.hpp>

using namespace Engine;

constructor_cpp(Texture)
{}

static ObjID _M_current_binded = 0;

declare_instance_info_cpp(Texture);

Texture::Texture(const TextureParams& params)
{
    create(params);
}

Texture& Texture::create(const TextureParams& params)
{
    ApiObject::destroy();
    EngineInstance::instance()->api_interface()->create_texture(_M_ID, params);
    return *this;
}

const Texture& Texture::bind(unsigned int num) const
{
    if (_M_ID == _M_current_binded)
        return *this;
    _M_current_binded = _M_ID;
    EngineInstance::instance()->api_interface()->bind_texture(_M_ID, num);
    return *this;
}

int Texture::base_level() const
{
    return EngineInstance::instance()->api_interface()->base_level_texture(_M_ID);
}

Texture& Texture::base_level(int level)
{
    EngineInstance::instance()->api_interface()->base_level_texture(_M_ID, level);
    return *this;
}

Texture& Texture::depth_stencil_mode(DepthStencilMode mode)
{
    EngineInstance::instance()->api_interface()->depth_stencil_mode_texture(_M_ID, mode);
    return *this;
}

DepthStencilMode Texture::depth_stencil_mode() const
{
    return EngineInstance::instance()->api_interface()->depth_stencil_mode_texture(_M_ID);
}

CompareFunc Texture::compare_func() const
{
    return EngineInstance::instance()->api_interface()->compare_func_texture(_M_ID);
}

Texture& Texture::compare_func(CompareFunc func)
{
    EngineInstance::instance()->api_interface()->compare_func_texture(_M_ID, func);
    return *this;
}

Texture& Texture::compare_mode(CompareMode mode)
{
    EngineInstance::instance()->api_interface()->compare_mode_texture(_M_ID, mode);
    return *this;
}

CompareMode Texture::compare_mode() const
{
    return EngineInstance::instance()->api_interface()->compare_mode_texture(_M_ID);
}

const TextureFilter Texture::min_filter() const
{
    return EngineInstance::instance()->api_interface()->min_filter_texture(_M_ID);
}

const TextureFilter Texture::mag_filter() const
{
    return EngineInstance::instance()->api_interface()->mag_filter_texture(_M_ID);
}

Texture& Texture::min_filter(TextureFilter filter)
{
    EngineInstance::instance()->api_interface()->min_filter_texture(_M_ID, filter);
    return *this;
}

Texture& Texture::mag_filter(TextureFilter filter)
{
    EngineInstance::instance()->api_interface()->mag_filter_texture(_M_ID, filter);
    return *this;
}

Texture& Texture::min_lod_level(int value)
{
    EngineInstance::instance()->api_interface()->min_lod_level_texture(_M_ID, value);
    return *this;
}

Texture& Texture::max_lod_level(int value)
{
    EngineInstance::instance()->api_interface()->max_lod_level_texture(_M_ID, value);
    return *this;
}

Texture& Texture::max_mipmap_level(int value)
{
    EngineInstance::instance()->api_interface()->max_mipmap_level_texture(_M_ID, value);
    return *this;
}

int Texture::min_lod_level() const
{
    return EngineInstance::instance()->api_interface()->min_lod_level_texture(_M_ID);
}

int Texture::max_lod_level() const
{
    return EngineInstance::instance()->api_interface()->max_lod_level_texture(_M_ID);
}

int Texture::max_mipmap_level() const
{
    return EngineInstance::instance()->api_interface()->max_mipmap_level_texture(_M_ID);
}

SwizzleRGBA Texture::swizzle() const
{
    return EngineInstance::instance()->api_interface()->swizzle_texture(_M_ID);
}

Texture& Texture::swizzle(const SwizzleRGBA& value)
{
    EngineInstance::instance()->api_interface()->swizzle_texture(_M_ID, value);
    return *this;
}

Texture& Texture::wrap_s(const WrapValue& wrap)
{
    EngineInstance::instance()->api_interface()->wrap_s_texture(_M_ID, wrap);
    return *this;
}

Texture& Texture::wrap_t(const WrapValue& wrap)
{
    EngineInstance::instance()->api_interface()->wrap_t_texture(_M_ID, wrap);
    return *this;
}

Texture& Texture::wrap_r(const WrapValue& wrap)
{
    EngineInstance::instance()->api_interface()->wrap_r_texture(_M_ID, wrap);
    return *this;
}

WrapValue Texture::wrap_s() const
{
    return EngineInstance::instance()->api_interface()->wrap_s_texture(_M_ID);
}

WrapValue Texture::wrap_t() const
{
    return EngineInstance::instance()->api_interface()->wrap_t_texture(_M_ID);
}

WrapValue Texture::wrap_r() const
{
    return EngineInstance::instance()->api_interface()->wrap_r_texture(_M_ID);
}

Texture& Texture::generate_mipmap()
{
    EngineInstance::instance()->api_interface()->generate_texture_mipmap(_M_ID);
    return *this;
}

Size3D Texture::size(int level) const
{
    Size3D _M_size;
    EngineInstance::instance()->api_interface()->texture_size(_M_ID, _M_size, level);
    return _M_size;
}

ObjID Texture::internal_id() const
{
    return EngineInstance::instance()->api_interface()->texture_id(_M_ID);
}
