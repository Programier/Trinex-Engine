#pragma once
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
    class ENGINE_EXPORT Texture2D : public Texture
    {
        declare_class(Texture2D, Texture);

    public:
        Image image;
        Path path;

        Texture2D();
        delete_copy_constructors(Texture2D);
        Texture2D& rhi_create() override;
        Texture2D& rhi_create(const byte* data);
        Texture2D& update(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap = 0, const byte* data = nullptr);
        Texture2D& read_data(Buffer& data, MipMapLevel level = 0);
        Texture2D& read_image(Image& image, MipMapLevel level = 0);
        TextureType type() const override;
        Texture2D& apply_changes() override;

        bool archive_process(Archive& archive) override;
    };
}// namespace Engine
