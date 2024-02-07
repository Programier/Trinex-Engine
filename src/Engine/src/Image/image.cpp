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
    bool Image::empty() const
    {
        return _M_data.empty() || _M_channels == 0 || _M_width == 0 || _M_height == 0;
    }

    byte* Image::data()
    {
        return _M_data.data();
    }

    const byte* Image::data() const
    {
        return _M_data.data();
    }

    Buffer& Image::buffer()
    {
        return _M_data;
    }

    const Buffer& Image::buffer() const
    {
        return _M_data;
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


    Image& Image::load(const Path& image, const bool& invert_horizontal)
    {
        _M_data.clear();
        _M_data.shrink_to_fit();
        stbi_set_flip_vertically_on_load(invert_horizontal);
        throw not_implemented; // TODO: Need rewrite for use with VFS
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

    Image& Image::load_from_memory(const byte* buffer, size_t size)
    {
        _M_data.clear();
        _M_data.shrink_to_fit();

        stbi_uc* image_data = stbi_load_from_memory(buffer, static_cast<int>(size), &_M_width, &_M_height, &_M_channels, 0);
        _M_data.resize(_M_width * _M_height * _M_channels);
        std::copy(image_data, image_data + _M_data.size(), _M_data.data());
        stbi_image_free(image_data);

        return *this;
    }

    Image& Image::load_from_memory(const Buffer& buffer)
    {
        return load_from_memory(buffer.data(), buffer.size());
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


    Image::~Image()
    {}


    Image::Image(Image&& img)
    {
        *this = std::move(img);
    }

    Image& Image::operator=(Image&& img)
    {
        if (this == &img)
            return *this;

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

        auto status = stbir_resize_uint8(_M_data.data(), _M_width, _M_height, _M_width * _M_channels, resized_image.data(),
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
        return static_cast<bool>(
                stbi_write_png(filename.c_str(), _M_width, _M_height, _M_channels, _M_data.data(), _M_width * _M_channels));
    }

    bool Image::write_jpg(const String& filename)
    {
        return static_cast<bool>(stbi_write_jpg(filename.c_str(), _M_width, _M_height, _M_channels, _M_data.data(), 100));
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

        static bool (Engine::Image::*write_methods[])(const String& f) = {&Image::write_png, &Image::write_jpg, &Image::write_bmp,
                                                                          &Image::write_tga};

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

    bool Image::archive_process(Archive& archive)
    {
        if (!SerializableObject::archive_process(archive))
        {
            return false;
        }

        //        if (archive.is_saving() && _M_data.empty())
        //        {
        //            error_log("Image", "Failed to serialize image. Data is empty!");
        //            return false;
        //        }

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
            size_t size = static_cast<size_t>(_M_width) * static_cast<size_t>(_M_height) * static_cast<size_t>(_M_channels);

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
