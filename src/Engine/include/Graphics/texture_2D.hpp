#pragma once
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
    class ENGINE_EXPORT Texture2D : public Texture
    {
        declare_class(Texture2D, Texture);

    public:
        Texture2D();
        delete_copy_constructors(Texture2D);
        Texture2D& rhi_create() override;
        Texture2D& rhi_create(const byte* data);
        Texture2D& update(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap = 0,
                          const byte* data = nullptr);
        Texture2D& read_data(Buffer& data, MipMapLevel level = 0);
        Texture2D& load(const Image& image);
        Texture2D& load(const String& filename);
        bool load();
        Texture2D& read_image(Image& image, MipMapLevel level = 0);
        Image& resource_image(bool create = false);
        TextureType type() const override;

        bool archive_process(Archive* archive) override;
    };
}// namespace Engine
