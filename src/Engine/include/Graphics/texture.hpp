#pragma once
#include <Core/api_object.hpp>
#include <Core/resource.hpp>
#include <Core/rhi_initializers.hpp>


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
        TextureCreateInfo info;

    protected:
        TextureType _M_type = TextureType::Texture2D;

    public:
        Texture();
        delete_copy_constructors(Texture);

        Texture& create(const byte* data = nullptr);
        Identifier internal_id() const;
        const Texture& bind_combined(Sampler* sampler, BindingIndex binding, BindingIndex set = 0) const;
        Texture& generate_mipmap();

        Size2D size(MipMapLevel level = 0) const;
        bool archive_process(Archive* archive) override;
        ~Texture();
    };

}// namespace Engine
