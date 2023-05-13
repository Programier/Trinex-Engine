#pragma once

#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/serializable_object.hpp>
#include <string>


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
        void delete_image();

        bool write_png(const String& filename);
        bool write_jpg(const String& filename);
        bool write_bmp(const String& filename);
        bool write_tga(const String& filename);

    public:
        class ENGINE_EXPORT ImageRow
        {
            byte* _M_data     = nullptr;
            int_t _M_length   = 0;
            int_t _M_channels = 0;

        public:
            class ENGINE_EXPORT Pixel
            {
                byte* _M_data;
                int_t _M_channels;

            public:
                Pixel(byte* data, int_t channels);
                byte& R();
                byte& G();
                byte& B();
                byte& A();
                bool has_alpha();
                byte* begin();
                byte* end();

                Pixel& operator=(const Pixel& pixel);
            };
            ImageRow(byte* data, int_t length, int_t _M_channels);
            Pixel operator[](int_t index);
        };

        Image();
        Image(const String& path, const bool& invert_horizontal = false);
        Image(const Size2D& size, uint_t channels, const Buffer& buffer = {});
        Image(const Size2D& size, uint_t channels, const byte* data = {});
        Image(const Image&);
        Image& operator=(const Image&);
        Image(Image&&);
        Image& operator=(Image&&);

        const byte* data() const;
        Size1D width() const;
        Size1D height() const;
        uint_t channels() const;
        ImageRow operator[](int_t index);
        Image& load(const String& image, const bool& invert = false);
        Image& remove_alpha_channel();
        Image& add_alpha_channel();
        Buffer::iterator begin();
        Buffer::iterator end();
        Buffer::const_iterator begin() const;
        Buffer::const_iterator end() const;
        Buffer& vector();
        const Buffer& vector() const;
        Size2D size() const;
        bool resize(const Size2D& new_size);
        bool empty() const;
        bool save(String path, ImageType type = ImageType::PNG);
        Image& create(const Size2D& size, uint_t channels, const Buffer& buffer = {});
        Image& create(const Size2D& size, uint_t channels, const byte* buffer = nullptr);

        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;

        ~Image();
        Image sub_image(const Point2D& point, const Size2D& size);

        friend class ENGINE_EXPORT Texture2D;
    };
}// namespace Engine
