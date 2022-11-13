#pragma once
#include <Core/engine_types.hpp>
#include <Core/object.hpp>
#include <TemplateFunctional/reference_wrapper.hpp>

namespace Engine
{


    CLASS Texture : public Object
    {
    public:
        Texture();
        Texture(const TextureParams& params);
        Texture(const Texture& texture);
        Texture(Texture&& texture);
        Texture& operator=(const Texture& texture);
        Texture& operator=(Texture&& texture);

        Texture& create(const TextureParams& params);
        const Texture& bind(unsigned int num = 0) const;
        int base_level() const;
        Texture& base_level(int level);
        Texture& depth_stencil_mode(DepthStencilMode mode);
        DepthStencilMode depth_stencil_mode() const;
        CompareFunc compare_func() const;
        Texture& compare_func(CompareFunc func);
        Texture& compare_mode(CompareMode mode);
        CompareMode compare_mode() const;
        const TextureFilter min_filter() const;
        const TextureFilter mag_filter() const;
        Texture& min_filter(TextureFilter filter);
        Texture& mag_filter(TextureFilter filter);
        Texture& min_lod_level(int value);
        Texture& max_lod_level(int value);
        Texture& max_mipmap_level(int value);
        int min_lod_level() const;
        int max_lod_level() const;
        int max_mipmap_level() const;
        SwizzleRGBA swizzle() const;
        Texture& swizzle(const SwizzleRGBA& value);
        Texture& wrap_s(const WrapValue& wrap);
        Texture& wrap_t(const WrapValue& wrap);
        Texture& wrap_r(const WrapValue& wrap);
        WrapValue wrap_s() const;
        WrapValue wrap_t() const;
        WrapValue wrap_r() const;
        Texture& generate_mipmap();
        Size3D size(int level) const;
        ObjID internal_id() const;
    };

}// namespace Engine
