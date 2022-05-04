#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Engine
{

    class Image
    {
        std::vector<unsigned char> _M_data;
        int _M_height = 0, _M_width = 0, _M_channels = 0;
        void* _M_glfw_image = nullptr;
        void configure();

    public:
        class ImageRow
        {
            unsigned char* _M_data = nullptr;
            int _M_length = 0;
            int _M_channels = 0;

        public:
            class Pixel
            {
                unsigned char* _M_data;
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
            ImageRow(unsigned char* data, int length, int _M_channels);
            Pixel operator[](int index);
        };

        Image();
        Image(const std::string& path, const bool& invert_horizontal = false);
        Image(const Image&);
        Image& operator=(const Image&);
        const unsigned char* data() const;
        int width() const;
        int height() const;
        int channels() const;
        ImageRow operator[](int index);
        Image& load(const std::string& image, const bool& invert = false);
        void* glfw_image();
        Image& remove_alpha_channel();
        Image& add_alpha_channel();
        std::vector<unsigned char>::iterator begin();
        std::vector<unsigned char>::iterator end();
        std::vector<unsigned char>& vector();
        glm::vec<2, std::size_t, glm::defaultp> size() const;
        Image& size(const glm::vec<2, std::size_t, glm::defaultp>& _size);
        bool empty();
        ~Image();
        Image sub_image(const glm::vec2& begin, const glm::vec2& end);
    };
}// namespace Engine
