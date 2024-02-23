#pragma once

#include <Core/color_format.hpp>
#include <Core/engine_types.hpp>
#include <Core/serializable_object.hpp>


namespace Engine
{
    enum class ImageType : EnumerateType
    {
        PNG = 0,
        JPG = 1,
        BMP = 2,
        TGA = 3,
    };

    enum class ImageCompression : EnumerateType
    {
        None = 0,
        BC1  = 1,
        BC2  = 2,
        BC3  = 3,
        BC7  = 4,
    };

    class ENGINE_EXPORT Image : public SerializableObject
    {
        Buffer m_data;
        int_t m_height = 0, m_width = 0, m_channels = 0;
        ImageCompression m_compression = ImageCompression::None;

        bool write_png(const Path& filename);
        bool write_jpg(const Path& filename);
        bool write_bmp(const Path& filename);
        bool write_tga(const Path& filename);

    public:
        Image();
        Image(const Path& path, const bool& invert_horizontal = false);
        Image(const Size2D& size, uint_t channels, const Buffer& buffer = {});
        Image(const Size2D& size, uint_t channels, const byte* data = {});
        Image(const Image&);
        Image& operator=(const Image&);
        Image(Image&&);
        Image& operator=(Image&&);

        byte* data();
        const byte* data() const;
        Buffer& buffer();
        const Buffer& buffer() const;

        Size2D size() const;
        Size1D width() const;
        Size1D height() const;
        uint_t channels() const;
        bool resize(const Size2D& new_size);
        bool empty() const;
        ColorFormat format() const;

        Image& load(const Path& image, const bool& invert = false);
        Image& load_from_memory(const byte* buffer, size_t size);
        Image& load_from_memory(const Buffer& buffer);
        bool save(Path path, ImageType type = ImageType::PNG);

        Image& create(const Size2D& size, uint_t channels, const Buffer& buffer = {});
        Image& create(const Size2D& size, uint_t channels, const byte* buffer = nullptr);
        ImageCompression compression() const;
        void recompress(ImageCompression new_compression);
        ~Image();

        bool archive_process(Archive& archive) override;
        friend class ENGINE_EXPORT Texture2D;
    };
}// namespace Engine
