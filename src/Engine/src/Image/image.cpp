#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Image/image.hpp>
#include <stb_dxt.h>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <stdexcept>

namespace Engine
{
    bool Image::empty() const
    {
        return m_data.empty() || m_channels == 0 || m_width == 0 || m_height == 0;
    }

    byte* Image::data()
    {
        return m_data.data();
    }

    const byte* Image::data() const
    {
        return m_data.data();
    }

    Buffer& Image::buffer()
    {
        return m_data;
    }

    const Buffer& Image::buffer() const
    {
        return m_data;
    }

    Size1D Image::width() const
    {
        return static_cast<Size1D>(m_width);
    }

    Size1D Image::height() const
    {
        return static_cast<Size1D>(m_height);
    }

    Size2D Image::size() const
    {
        return {width(), height()};
    }


    uint_t Image::channels() const
    {
        return m_channels;
    }


    Image& Image::load(const Path& image, const bool& invert_horizontal)
    {
        m_data.clear();
        m_data.shrink_to_fit();

        Buffer buffer = FileReader(image).read_buffer();
        return load_from_memory(buffer);
    }

    Image& Image::load_from_memory(const byte* buffer, size_t size)
    {
        m_data.clear();
        m_data.shrink_to_fit();

        stbi_uc* image_data = stbi_load_from_memory(buffer, static_cast<int>(size), &m_width, &m_height, &m_channels, 0);
        m_data.resize(m_width * m_height * m_channels);
        std::copy(image_data, image_data + m_data.size(), m_data.data());
        stbi_image_free(image_data);
        m_compression = ImageCompression::None;

        return *this;
    }

    Image& Image::load_from_memory(const Buffer& buffer)
    {
        return load_from_memory(buffer.data(), buffer.size());
    }

    Image::Image() = default;

    Image::Image(const Path& path, const bool& invert_horizontal)
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
        m_data     = img.m_data;
        m_width    = img.m_width;
        m_height   = img.m_height;
        m_channels = img.m_channels;
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

        m_data     = std::move(img.m_data);
        m_channels = img.m_channels;
        m_width    = img.m_width;
        m_height   = img.m_height;

        img.m_channels = img.m_height = img.m_width = 0;
        return *this;
    }

    bool Image::resize(const Size2D& new_size)
    {
        int new_width  = static_cast<int>(new_size.x);
        int new_height = static_cast<int>(new_size.y);

        Vector<byte> resized_image(new_width * new_height * m_channels, 0);

        auto status = stbir_resize_uint8(m_data.data(), m_width, m_height, m_width * m_channels, resized_image.data(), new_width,
                                         new_height, new_width * m_channels, m_channels);

        m_width  = new_width;
        m_height = new_height;
        m_data   = std::move(resized_image);
        return status;
    }

    static String extension_of_type(ImageType type)
    {
        static String extensions[] = {".png", ".jpg", ".bmp", ".tga"};
        return extensions[static_cast<EnumerateType>(type)];
    }

    static void image_writer_func(void* context, void* data, int size)
    {
        FileWriter* writer = reinterpret_cast<FileWriter*>(context);
        writer->write(reinterpret_cast<const byte*>(data), size);
    }

#define make_writer()                                                                                                            \
    FileWriter writer(filename);                                                                                                 \
    if (!writer.is_open())                                                                                                       \
        return false;

    bool Image::write_png(const Path& filename)
    {
        make_writer();
        return static_cast<bool>(stbi_write_png_to_func(image_writer_func, &writer, m_width, m_height, m_channels, m_data.data(),
                                                        m_width * m_channels));
    }

    bool Image::write_jpg(const Path& filename)
    {
        make_writer();
        return static_cast<bool>(
                stbi_write_jpg_to_func(image_writer_func, &writer, m_width, m_height, m_channels, m_data.data(), 100));
    }

    bool Image::write_bmp(const Path& filename)
    {
        return false;
    }

    bool Image::write_tga(const Path& filename)
    {
        return false;
    }

    bool Image::save(Path path, ImageType type)
    {
        path += extension_of_type(type);

        static bool (Engine::Image::*write_methods[])(const Path& f) = {&Image::write_png, &Image::write_jpg, &Image::write_bmp,
                                                                        &Image::write_tga};

        auto method = write_methods[static_cast<EnumerateType>(type)];
        return ((*this).*method)(path);
    }

    Image& Image::create(const Size2D& size, uint_t channels, const Buffer& buffer)
    {
        m_channels = channels;
        m_width    = static_cast<int_t>(size.x);
        m_height   = static_cast<int_t>(size.y);

        m_data.resize(m_channels * m_width * m_height);
        std::copy(buffer.begin(), buffer.end(), m_data.begin());

        return *this;
    }

    Image& Image::create(const Size2D& size, uint_t channels, const byte* buffer)
    {
        m_channels = channels;
        m_width    = static_cast<int_t>(size.x);
        m_height   = static_cast<int_t>(size.y);

        auto buffer_size = m_width * m_height * m_channels;
        m_data.reserve(buffer_size);
        if (buffer)
            std::copy(buffer, buffer + buffer_size, m_data.data());
        else
            std::fill(m_data.begin(), m_data.end(), 0);
        return *this;
    }

    ImageCompression Image::compression() const
    {
        return m_compression;
    }

    void Image::recompress(ImageCompression new_compression)
    {
        m_compression = new_compression;
    }

    ColorFormat Image::format() const
    {
        if(m_compression == ImageCompression::BC7)
            return ColorFormat::BC7Unorm;

        if (m_channels == 1)
        {
            return ColorFormat::R8Unorm;
        }

        if (m_channels == 2)
        {
            return ColorFormat::R8G8Unorm;
        }

        if (m_channels == 3)
        {
            return ColorFormat::R8G8B8Unorm;
        }

        if (m_channels == 4)
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

        //        if (archive.is_saving() && m_data.empty())
        //        {
        //            error_log("Image", "Failed to serialize image. Data is empty!");
        //            return false;
        //        }

        archive & m_width;
        archive & m_height;
        archive & m_channels;

        if (!archive)
        {
            error_log("Image", "Failed to serialize image header!");
            return false;
        }

        if (archive.is_reading())
        {
            m_data.clear();
            size_t size = static_cast<size_t>(m_width) * static_cast<size_t>(m_height) * static_cast<size_t>(m_channels);

            m_data.resize(size);

            if (!archive.reader()->read(m_data.data(), m_data.size()))
            {
                error_log("Image", "Failed to serialize image data!");
                return false;
            }
        }
        else
        {
            if (!archive.writer()->write(m_data.data(), m_data.size()))
            {
                error_log("Image", "Failed to serialize image data!");
                return false;
            }
        }

        return static_cast<bool>(archive);
    }
}// namespace Engine
