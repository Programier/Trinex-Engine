#pragma once
#include <Core/api_object.hpp>
#include <Core/color_format.hpp>
#include <Core/resource.hpp>


namespace Engine
{
    class Sampler;

    struct ENGINE_EXPORT TextureResources : public SerializableObject {
        Vector<class Image> images;

    public:
        bool archive_process(Archive* archive) override;
        friend class Texture;
        friend class Object;
    };


    class ENGINE_EXPORT Texture : public Resource<TextureResources, ApiBindingObject>
    {
        declare_class(Texture, ApiObject);

    public:
        Size2D size                = {1, 1};
        MipMapLevel base_mip_level = 0;
        MipMapLevel mipmap_count   = 1;
        ColorFormat format         = ColorFormat::R8G8B8A8Unorm;
        SwizzleRGBA swizzle;

    protected:
        bool _M_use_for_render_target = false;

    public:
        Texture();
        delete_copy_constructors(Texture);
        const Texture& bind_combined(Sampler* sampler, BindLocation location) const;
        Texture& generate_mipmap();
        Texture& setup_render_target_texture();
        bool is_render_target_texture() const;


        Size2D mip_size(MipMapLevel level = 0) const;
        bool archive_process(Archive* archive) override;
        virtual TextureType type() const = 0;
        ~Texture();
    };

}// namespace Engine
