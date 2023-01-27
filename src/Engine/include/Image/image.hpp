#pragma once

#include <Core/engine_types.hpp>
#include <string>
#include <vector>
#include <Core/export.hpp>

namespace Engine
{

    CLASS Image
    {
        std::vector<byte> _M_data;
        int_t _M_height = 0, _M_width = 0, _M_channels = 0;

        void delete_image();

    public:
        class ImageRow
        {
            byte* _M_data = nullptr;
            int_t _M_length = 0;
            int_t _M_channels = 0;

        public:
            class Pixel
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
        Image(const Image&);
        Image& operator=(const Image&);
        Image(Image&&);
        Image& operator=(Image&&);

        const byte* data() const;
        Size1D width() const;
        Size1D height() const;
        Size1D channels() const;
        ImageRow operator[](int_t index);
        Image& load(const String& image, const bool& invert = false);
        Image& remove_alpha_channel();
        Image& add_alpha_channel();
        std::vector<byte>::iterator begin();
        std::vector<byte>::iterator end();
        std::vector<byte>& vector();
        Size2D size() const;
        Image& size(const Size2D& _size);
        bool empty() const;
        ~Image();
        Image sub_image(const glm::vec2& begin, const glm::vec2& end);
    };
}// namespace Engine
