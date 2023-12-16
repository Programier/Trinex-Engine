#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Image/image.hpp>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
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

    const byte* Image::data() const
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


    uint_t Image::channels() const
    {
        return _M_channels;
    }

    Image::ImageRow Image::operator[](int_t index)
    {

        if (index >= _M_height || index < -_M_height)
            EngineException("Index out of range");
        index = index < 0 ? _M_height - index : index;
        return Image::ImageRow(_M_data.data() + _M_channels * index, _M_width, _M_channels);
    }

    Image& Image::load(const String& image, const bool& invert_horizontal)
    {
        _M_data.clear();
        stbi_set_flip_vertically_on_load(invert_horizontal);
        byte* address = stbi_load(image.c_str(), &_M_width, &_M_height, &_M_channels, 0);
        stbi_set_flip_vertically_on_load(false);

        if (address == nullptr)
        {
            return *this;
        }

        _M_data.insert(_M_data.begin(), address, address + _M_width * _M_height * _M_channels);
        stbi_image_free(address);
        return *this;
    }

    Image::Image() = default;

    Image::Image(const String& path, const bool& invert_horizontal)
    {
        load(path, invert_horizontal);
    }

    Image::Image(const Size2D& size, uint_t channels, const Buffer& buffer)
    {
        create(size, channels, buffer);
    }

    Image::Image(const Size2D& size, uint_t channels, const byte* data)
    {
        create(size, channels, data);
    }

    Image::Image(const Image& img)
    {
        *this = img;
    }

    Image& Image::operator=(const Image& img)
    {
        if (this == &img)
            return *this;
        _M_data     = img._M_data;
        _M_width    = img._M_width;
        _M_height   = img._M_height;
        _M_channels = img._M_channels;
        return *this;
    }

    Image& Image::remove_alpha_channel()
    {

        if (_M_channels == 3)
            return *this;
        if (_M_channels != 4)
        {
            info_log("Image", "Cannot remove alpha channel\n");
            return *this;
        }

        Vector<byte> tmp;
        tmp.reserve(_M_height * _M_width * 3);
        int_t len = _M_width * _M_height * _M_channels;

        for (int_t i = 0; i < len; i++)
        {
            if (i % 4 != 3)
                tmp.push_back(_M_data[i]);
        }

        _M_data     = std::move(tmp);
        _M_channels = 3;
        return *this;
    }

    Image& Image::add_alpha_channel()
    {

        if (_M_channels == 4)
            return *this;
        if (_M_channels != 3)
        {
            info_log("Image", "Cannot add alpha channel\n");
            return *this;
        }

        Vector<byte> tmp;
        tmp.reserve(_M_height * _M_width * 4);
        int_t len = _M_width * _M_height * _M_channels;
        for (int_t i = 0; i < len; i++)
        {
            tmp.push_back(_M_data[i]);
            if (i % 3 == 2)
                tmp.push_back(255);
        }
        _M_data     = std::move(tmp);
        _M_channels = 4;
        return *this;
    }

    Vector<byte>& Image::vector()
    {
        return _M_data;
    }

    const Buffer& Image::vector() const
    {
        return _M_data;
    }

    Buffer::iterator Image::begin()
    {
        return _M_data.begin();
    }

    Buffer::iterator Image::end()
    {
        return _M_data.end();
    }

    Buffer::const_iterator Image::begin() const
    {
        return _M_data.begin();
    }

    Buffer::const_iterator Image::end() const
    {
        return _M_data.end();
    }


    Image::~Image()
    {
        delete_image();
    }


    //          IMAGE ROW
    Image::ImageRow::ImageRow(byte* data, int_t length, int_t channels)
        : _M_data(data), _M_length(length), _M_channels(channels)
    {}

    Image::ImageRow::Pixel Image::ImageRow::operator[](int_t index)
    {
        if (index >= _M_length || index < -_M_length)
            EngineException("Index out of range");
        index = index < 0 ? _M_length - index : index;
        return Pixel(_M_data + index * _M_channels, _M_channels);
    }

    //          PIXEL

    Image::ImageRow::Pixel::Pixel(byte* data, int_t channels) : _M_data(data), _M_channels(channels)
    {}

    byte& Image::ImageRow::Pixel::R()
    {
        return _M_data[0];
    }

    byte& Image::ImageRow::Pixel::G()
    {
        return _M_data[1];
    }

    byte& Image::ImageRow::Pixel::B()
    {
        return _M_data[2];
    }

    byte& Image::ImageRow::Pixel::A()
    {
        if (has_alpha() == false)
            throw std::runtime_error("Pixel: Alpha channel not found");
        return _M_data[3];
    }

    bool Image::ImageRow::Pixel::has_alpha()
    {
        return _M_channels == 4;
    }

    byte* Image::ImageRow::Pixel::begin()
    {
        return _M_data;
    }

    byte* Image::ImageRow::Pixel::end()
    {
        return _M_data + _M_channels;
    }

    Image::ImageRow::Pixel& Image::ImageRow::Pixel::operator=(const Pixel& pixel)
    {
        std::copy(pixel._M_data, pixel._M_data + std::min(_M_channels, pixel._M_channels), _M_data);

        return *this;
    }

    Image Image::sub_image(const Point2D& begin, const Size2D& size)
    {
        Image image;
        auto end = begin + size;

        if (size.x < 0 || size.y < 0)
            throw EngineException("Imcorrect index");
        if (end.x > _M_width || begin.x > _M_width || end.y > _M_height || begin.y > _M_height)
            throw EngineException("Index out of range");

        image._M_width    = static_cast<int>(size.x + 0.5);
        image._M_height   = static_cast<int>(size.y + 0.5);
        image._M_channels = _M_channels;

        image._M_data.reserve(image._M_width * image._M_height * _M_channels);

        int_t start_index = (_M_width * _M_channels * static_cast<int>(begin.y + 0.5)) +
                            static_cast<int>(begin.x + 0.5) * _M_channels;
        for (int_t i = 0; i < image._M_height; i++)
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
        _M_data     = std::move(img._M_data);
        _M_channels = img._M_channels;
        _M_width    = img._M_width;
        _M_height   = img._M_height;

        img._M_channels = img._M_height = img._M_width = 0;
        return *this;
    }

    bool Image::resize(const Size2D& new_size)
    {
        int new_width  = static_cast<int>(new_size.x);
        int new_height = static_cast<int>(new_size.y);

        Vector<byte> resized_image(new_width * new_height * _M_channels, 0);

        auto status =
                stbir_resize_uint8(_M_data.data(), _M_width, _M_height, _M_width * _M_channels, resized_image.data(),
                                   new_width, new_height, new_width * _M_channels, _M_channels);

        _M_width  = new_width;
        _M_height = new_height;
        _M_data   = std::move(resized_image);
        return status;
    }

    static String extension_of_type(ImageType type)
    {
        static String extensions[] = {".png", ".jpg", ".bmp", ".tga"};
        return extensions[static_cast<EnumerateType>(type)];
    }

    bool Image::write_png(const String& filename)
    {
        return static_cast<bool>(stbi_write_png(filename.c_str(), _M_width, _M_height, _M_channels, _M_data.data(),
                                                _M_width * _M_channels));
    }

    bool Image::write_jpg(const String& filename)
    {
        return static_cast<bool>(
                stbi_write_jpg(filename.c_str(), _M_width, _M_height, _M_channels, _M_data.data(), 100));
    }

    bool Image::write_bmp(const String& filename)
    {
        return false;
    }

    bool Image::write_tga(const String& filename)
    {
        return false;
    }

    bool Image::save(String path, ImageType type)
    {
        path += extension_of_type(type);

        static bool (Engine::Image::*write_methods[])(const String& f) = {&Image::write_png, &Image::write_jpg,
                                                                          &Image::write_bmp, &Image::write_tga};

        auto method = write_methods[static_cast<EnumerateType>(type)];
        return ((*this).*method)(path);
    }

    Image& Image::create(const Size2D& size, uint_t channels, const Buffer& buffer)
    {
        _M_channels = channels;
        _M_width    = static_cast<int_t>(size.x);
        _M_height   = static_cast<int_t>(size.y);

        _M_data.resize(_M_channels * _M_width * _M_height);
        std::copy(buffer.begin(), buffer.end(), _M_data.begin());

        return *this;
    }

    Image& Image::create(const Size2D& size, uint_t channels, const byte* buffer)
    {
        _M_channels = channels;
        _M_width    = static_cast<int_t>(size.x);
        _M_height   = static_cast<int_t>(size.y);

        auto buffer_size = _M_width * _M_height * _M_channels;
        _M_data.reserve(buffer_size);
        if (buffer)
            std::copy(buffer, buffer + buffer_size, _M_data.data());
        else
            std::fill(_M_data.begin(), _M_data.end(), 0);
        return *this;
    }

    ColorFormat Image::format() const
    {
        if (_M_channels == 1)
        {
            return ColorFormat::R8Unorm;
        }

        if (_M_channels == 2)
        {
            return ColorFormat::R8G8Unorm;
        }

        if (_M_channels == 3)
        {
            return ColorFormat::R8G8B8Unorm;
        }

        if (_M_channels == 4)
        {
            return ColorFormat::R8G8B8A8Unorm;
        }

        return ColorFormat::Undefined;
    }

    bool Image::archive_process(Archive* archive_ptr)
    {
        if (!SerializableObject::archive_process(archive_ptr))
        {
            return false;
        }

        Archive& archive = *archive_ptr;

        if (archive.is_saving() && _M_data.empty())
        {
            error_log("Image", "Failed to serialize image. Data is empty!");
            return false;
        }

        archive& _M_width;
        archive& _M_height;
        archive& _M_channels;

        if (!archive)
        {
            error_log("Image", "Failed to serialize image header!");
            return false;
        }

        if (archive.is_reading())
        {
            _M_data.clear();
            size_t size =
                    static_cast<size_t>(_M_width) * static_cast<size_t>(_M_height) * static_cast<size_t>(_M_channels);

            _M_data.resize(size);

            if (!archive.reader()->read(_M_data.data(), _M_data.size()))
            {
                error_log("Image", "Failed to serialize image data!");
                return false;
            }
        }
        else
        {
            if (!archive.writer()->write(_M_data.data(), _M_data.size()))
            {
                error_log("Image", "Failed to serialize image data!");
                return false;
            }
        }

        return static_cast<bool>(archive);
    }
}// namespace Engine
