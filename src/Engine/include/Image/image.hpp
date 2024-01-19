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

    class ENGINE_EXPORT Image : public SerializableObject
    {
        Buffer _M_data;
        int_t _M_height = 0, _M_width = 0, _M_channels = 0;

        bool write_png(const String& filename);
        bool write_jpg(const String& filename);
        bool write_bmp(const String& filename);
        bool write_tga(const String& filename);

    public:
        Image();
        Image(const String& path, const bool& invert_horizontal = false);
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
        Image& remove_alpha_channel();
        Image& add_alpha_channel();
        bool resize(const Size2D& new_size);
        bool empty() const;
        ColorFormat format() const;

        Image& load(const Path& image, const bool& invert = false);
        Image& load_from_memory(const byte* buffer, size_t size);
        Image& load_from_memory(const Buffer& buffer);
        bool save(String path, ImageType type = ImageType::PNG);

        Image& create(const Size2D& size, uint_t channels, const Buffer& buffer = {});
        Image& create(const Size2D& size, uint_t channels, const byte* buffer = nullptr);
        ~Image();

        bool archive_process(Archive& archive) override;
        friend class ENGINE_EXPORT Texture2D;
    };
}// namespace Engine
