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
        virtual Texture2D& rhi_create(const byte* data, size_t size);
        virtual Texture2D& update(const Size2D& size, const Offset2D& offset, const byte* data, size_t data_size, MipMapLevel mipmap = 0);
        TextureType type() const override;
        Texture2D& apply_changes() override;

        bool archive_process(Archive& archive) override;
    };
}// namespace Engine
