#pragma once

#include <BasicFunctional/engine_types.hpp>
#include <string>
#include <vector>

namespace Engine
{

    class Image
    {
        std::vector<byte> _M_data;
        int _M_height = 0, _M_width = 0, _M_channels = 0;
        void* _M_glfw_image = nullptr;
        void configure();
        void delete_image();

    public:
        class ImageRow
        {
            byte* _M_data = nullptr;
            int _M_length = 0;
            int _M_channels = 0;

        public:
            class Pixel
            {
                byte* _M_data;
                int _M_channels;

            public:
                Pixel(unsigned char* data, int channels);
                unsigned char& R();
                unsigned char& G();
                unsigned char& B();
                unsigned char& A();
                bool has_alpha();
                unsigned char* begin();
                unsigned char* end();

                Pixel& operator=(const Pixel& pixel);
            };
            ImageRow(byte* data, int length, int _M_channels);
            Pixel operator[](int index);
        };

        Image();
        Image(const std::string& path, const bool& invert_horizontal = false);
        Image(const Image&);
        Image& operator=(const Image&);
        Image(Image&&);
        Image& operator=(Image&&);

        const byte* data() const;
        Size1D width() const;
        Size1D height() const;
        Size1D channels() const;
        ImageRow operator[](int index);
        Image& load(const std::string& image, const bool& invert = false);
        void* glfw_image();
        Image& remove_alpha_channel();
        Image& add_alpha_channel();
        std::vector<byte>::iterator begin();
        std::vector<byte>::iterator end();
        std::vector<byte>& vector();
        Size2D size() const;
        Image& size(const Size2D& _size);
        bool empty();
        ~Image();
        Image sub_image(const glm::vec2& begin, const glm::vec2& end);
    };
}// namespace Engine
