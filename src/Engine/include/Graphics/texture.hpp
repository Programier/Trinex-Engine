#pragma once
#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/render_types.hpp>
#include <Core/resource.hpp>
#include <Core/texture_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT TextureResources : public SerializableObject {
        TextureCreateInfo info;
        Vector<class Image> images;

    public:
        bool archive_process(Archive* archive) override;
        friend class Texture;
        friend class Object;
    };


    class ENGINE_EXPORT Texture : public Resource<TextureResources, ApiObject>
    {
        declare_class(Texture, ApiObject);

    protected:
        TextureType _M_type = TextureType::Texture2D;
        PixelType _M_pixel_type;


    public:
        Texture();
        delete_copy_constructors(Texture);

        Texture& create();
        Identifier internal_id() const;
        const Texture& bind(TextureBindIndex num = 0) const;


        MipMapLevel base_level() const;
        Texture& base_level(MipMapLevel level);
        CompareFunc compare_func() const;
        Texture& compare_func(CompareFunc func);
        Texture& compare_mode(CompareMode mode);
        CompareMode compare_mode() const;
        const TextureFilter min_filter() const;
        const TextureFilter mag_filter() const;
        Texture& min_filter(TextureFilter filter);
        Texture& mag_filter(TextureFilter filter);
        Texture& min_lod_level(LodLevel value);
        Texture& max_lod_level(LodLevel value);
        LodLevel min_lod_level() const;
        LodLevel max_lod_level() const;
        MipMapLevel max_mipmap_level() const;
        SwizzleRGBA swizzle() const;
        Texture& swizzle(const SwizzleRGBA& value);
        Texture& wrap_s(const WrapValue& wrap);
        Texture& wrap_t(const WrapValue& wrap);
        Texture& wrap_r(const WrapValue& wrap);
        WrapValue wrap_s() const;
        WrapValue wrap_t() const;
        WrapValue wrap_r() const;
        Texture& generate_mipmap();
        Size2D size(MipMapLevel level) const;
        PixelType pixel_type();
        Texture& anisotropic_filtering(float value);
        float anisotropic_filtering();
        static float max_anisotropic_filtering();
        SamplerMipmapMode sample_mipmap_mode();
        Texture& sample_mipmap_mode(SamplerMipmapMode mode);
        LodBias lod_bias();
        Texture& lod_bias(LodBias bias);

        bool archive_process(Archive* archive) override;
        TextureCreateInfo& info(bool create = false);
        const TextureCreateInfo& info() const;
        ~Texture();
    };

}// namespace Engine
