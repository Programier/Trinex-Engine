#pragma once
#include <Core/enums.hpp>
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

    class ENGINE_EXPORT Image : public SerializableObject
    {
        Buffer m_data;
        int_t m_height = 0, m_width = 0, m_channels = 0;
        bool m_is_compressed = false;

        bool write_png(const Path& filename);
        bool write_jpg(const Path& filename);
        bool write_bmp(const Path& filename);
        bool write_tga(const Path& filename);

        void resize_channels(int_t new_channels);

    public:
        Image();
        Image(const Path& path, const bool& invert_horizontal = false);
        Image(const Size2D& size, uint_t channels, const Buffer& buffer);
        Image(const Size2D& size, uint_t channels, const byte* data);
        Image(ByteColor color, const Size2D& size, uint_t channels = 4);
        Image(const Size2D& size, uint_t channels = 4);

        Image(const Image&);
        Image& operator=(const Image&);
        Image(Image&&);
        Image& operator=(Image&&);

        byte* data();
        const byte* data() const;
        Buffer& buffer();
        const Buffer& buffer() const;

        Size2D size() const;
        uint_t width() const;
        uint_t height() const;
        uint_t channels() const;
        bool resize(const Size2D& new_size);
        bool empty() const;
        ColorFormat format() const;

        Image& load(const Path& image, bool invert = false);
        Image& load_from_memory(const byte* buffer, size_t size, bool invert = false);
        Image& load_from_memory(const Buffer& buffer, bool invert = false);
        bool save(Path path, ImageType type = ImageType::PNG, bool invert = false);

        Image& create(const Size2D& size, uint_t channels, Buffer&& buffer);
        Image& create(const Size2D& size, uint_t channels, const Buffer& buffer);
        Image& create(const Size2D& size, uint_t channels, const byte* buffer);
        Image& create(ByteColor color, const Size2D& size, uint_t channels = 4);
        Image& create(const Size2D& size, uint_t channels = 4);
        Image& create_interface(const Size2D& size, uint_t channels = 4);

        byte* pixel_at(uint_t x, uint_t y);
        const byte* pixel_at(uint_t x, uint_t y) const;

        bool is_compressed() const;
        void compress();
        ~Image();

        bool archive_process(Archive& archive) override;
        friend class ENGINE_EXPORT Texture2D;
    };
}// namespace Engine
