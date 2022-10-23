#include <Core/logger.hpp>
#include <Image/image.hpp>
#include <stb_image.h>
#include <stdexcept>

namespace Engine
{

    void Image::delete_image()
    {
        _M_height = 0, _M_width = 0, _M_channels = 0;
    }

    bool Image::empty() const
    {
        return _M_data.empty() || _M_channels == 0 || _M_width == 0 || _M_height == 0;
    }

    const unsigned char* Image::data() const
    {
        return _M_data.data();
    }

    Size1D Image::width() const
    {
        return static_cast<Size1D>(_M_width);
    }

    Size1D Image::height() const
    {
        return static_cast<Size1D>(_M_height);
    }

    Size2D Image::size() const
    {
        return {width(), height()};
    }

    Image& Image::size(const Size2D& _size)
    {
        if (_size.x * _size.y * _M_channels >= _M_data.size())
            throw std::runtime_error("Image: Size out of range");
        _M_width = static_cast<int>(_size.x);
        _M_height = static_cast<int>(_size.y);
        return *this;
    }

    Size1D Image::channels() const
    {
        return _M_channels;
    }

    Image::ImageRow Image::operator[](int index)
    {

        if (index >= _M_height || index < -_M_height)
            std::runtime_error("Image: Index out of range");
        index = index < 0 ? _M_height - index : index;
        return Image::ImageRow(_M_data.data() + _M_channels * index, _M_width, _M_channels);
    }

    Image& Image::load(const std::string& image, const bool& invert_horizontal)
    {
        _M_data.clear();
        stbi_set_flip_vertically_on_load(invert_horizontal);
        unsigned char* address = stbi_load(image.c_str(), &_M_width, &_M_height, &_M_channels, 0);
        stbi_set_flip_vertically_on_load(false);
        //_M_data = std::vector<unsigned char>(address, address + _M_width * _M_height * _M_channels);
        _M_data.insert(_M_data.begin(), address, address + _M_width * _M_height * _M_channels);
        stbi_image_free(address);
        return *this;
    }

    Image::Image() = default;

    Image::Image(const std::string& path, const bool& invert_horizontal)
    {
        load(path, invert_horizontal);
    }

    Image::Image(const Image& img)
    {
        *this = img;
    }

    Image& Image::operator=(const Image& img)
    {
        if (this == &img)
            return *this;
        _M_data = img._M_data;
        _M_width = img._M_width;
        _M_height = img._M_height;
        _M_channels = img._M_channels;
        return *this;
    }

    Image& Image::remove_alpha_channel()
    {

        if (_M_channels == 3)
            return *this;
        if (_M_channels != 4)
        {
            logger->log("Image: Cannot remove alpha channel\n");
            return *this;
        }

        std::vector<unsigned char> tmp;
        tmp.reserve(_M_height * _M_width * 3);
        int len = _M_width * _M_height * _M_channels;
        for (int i = 0; i < len; i++)
        {
            if (i % 4 != 3)
                tmp.push_back(_M_data[i]);
        }
        _M_data = tmp;
        _M_channels = 3;
        return *this;
    }

    Image& Image::add_alpha_channel()
    {

        if (_M_channels == 4)
            return *this;
        if (_M_channels != 3)
        {
            logger->log("Image: Cannot add alpha channel\n");
            return *this;
        }

        std::vector<unsigned char> tmp;
        tmp.reserve(_M_height * _M_width * 4);
        int len = _M_width * _M_height * _M_channels;
        for (int i = 0; i < len; i++)
        {
            tmp.push_back(_M_data[i]);
            if (i % 3 == 2)
                tmp.push_back(255);
        }
        _M_data = tmp;
        _M_channels = 4;
        return *this;
    }

    std::vector<unsigned char>& Image::vector()
    {
        return _M_data;
    }

    std::vector<unsigned char>::iterator Image::begin()
    {
        return _M_data.begin();
    }

    std::vector<unsigned char>::iterator Image::end()
    {
        return _M_data.end();
    }

    Image::~Image()
    {
        delete_image();
    }


    //          IMAGE ROW
    Image::ImageRow::ImageRow(unsigned char* data, int length, int channels)
        : _M_data(data), _M_length(length), _M_channels(channels)
    {}

    Image::ImageRow::Pixel Image::ImageRow::operator[](int index)
    {
        if (index >= _M_length || index < -_M_length)
            std::runtime_error("Image: Index out of range");
        index = index < 0 ? _M_length - index : index;
        return Pixel(_M_data + index * _M_channels, _M_channels);
    }

    //          PIXEL

    Image::ImageRow::Pixel::Pixel(unsigned char* data, int channels) : _M_data(data), _M_channels(channels)
    {}

    unsigned char& Image::ImageRow::Pixel::R()
    {
        return _M_data[0];
    }

    unsigned char& Image::ImageRow::Pixel::G()
    {
        return _M_data[1];
    }

    unsigned char& Image::ImageRow::Pixel::B()
    {
        return _M_data[2];
    }

    unsigned char& Image::ImageRow::Pixel::A()
    {
        if (has_alpha() == false)
            throw std::runtime_error("Pixel: Alpha channel not found");
        return _M_data[3];
    }

    bool Image::ImageRow::Pixel::has_alpha()
    {
        return _M_channels == 4;
    }

    unsigned char* Image::ImageRow::Pixel::begin()
    {
        return _M_data;
    }

    unsigned char* Image::ImageRow::Pixel::end()
    {
        return _M_data + _M_channels;
    }

    Image::ImageRow::Pixel& Image::ImageRow::Pixel::operator=(const Pixel& pixel)
    {
        std::copy(pixel._M_data, pixel._M_data + std::min(_M_channels, pixel._M_channels), _M_data);

        return *this;
    }

    Image Image::sub_image(const glm::vec2& begin, const glm::vec2& end)
    {
        Image image;
        auto size = end - begin;
        if (size.x < 0 || size.y < 0)
            throw std::runtime_error("Image: Imcorrect index");
        if (end.x > _M_width || begin.x > _M_width || end.y > _M_height || begin.y > _M_height)
            throw std::runtime_error("Image: Index out of range");

        image._M_width = static_cast<int>(size.x + 0.5);
        image._M_height = static_cast<int>(size.y + 0.5);
        image._M_channels = _M_channels;

        image._M_data.reserve(image._M_width * image._M_height * _M_channels);

        int start_index =
                (_M_width * _M_channels * static_cast<int>(begin.y + 0.5)) + static_cast<int>(begin.x + 0.5) * _M_channels;
        for (int i = 0; i < image._M_height; i++)
        {
            image._M_data.insert(image.end(), _M_data.begin() + start_index,
                                 _M_data.begin() + start_index + image._M_width * image._M_channels);
            start_index += _M_width * _M_channels;
        }


        return image;
    }

    Image::Image(Image&& img)
    {
        *this = std::move(img);
    }

    Image& Image::operator=(Image&& img)
    {
        if (this == &img)
            return *this;

        delete_image();
        _M_data = std::move(img._M_data);
        _M_channels = img._M_channels;
        _M_width = img._M_width;
        _M_height = img._M_height;

        img._M_channels = img._M_height = img._M_width = 0;
        return *this;
    }


}// namespace Engine
