#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <Image/image.hpp>
#include <stb_dxt.h>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

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

	uint_t Image::width() const
	{
		return static_cast<uint_t>(m_width);
	}

	uint_t Image::height() const
	{
		return static_cast<uint_t>(m_height);
	}

	Size2D Image::size() const
	{
		return {width(), height()};
	}


	uint_t Image::channels() const
	{
		return m_channels;
	}

	void Image::resize_channels(int_t new_channels)
	{
		if (new_channels < 1 || new_channels > 4)
			return;

		int_t channels_diff = new_channels - m_channels;

		if (channels_diff != 0)
		{
			int_t pixels = m_width * m_height;
			Buffer new_data(m_data.size() + (pixels * channels_diff), 255);

			int_t min_channels = glm::min(new_channels, m_channels);

			byte* src_memory = m_data.data();
			byte* dst_memory = new_data.data();

			for (int_t pixel = 0; pixel < pixels; pixel++)
			{
				byte* src_pixel_data = src_memory + (pixel * m_channels);
				byte* dst_pixel_data = dst_memory + (pixel * new_channels);

				for (int_t channel = 0; channel < min_channels; channel++)
				{
					dst_pixel_data[channel] = src_pixel_data[channel];
				}
			}

			m_data     = std::move(new_data);
			m_channels = new_channels;
		}
	}


	Image& Image::load(const Path& image, bool invert_horizontal)
	{
		m_data.clear();
		m_data.shrink_to_fit();

		Buffer buffer = FileReader(image).read_buffer();
		load_from_memory(buffer, invert_horizontal);

		if (m_channels > 1)
		{
			resize_channels(4);
		}
		return *this;
	}


	Image& Image::load_from_memory(const byte* buffer, size_t size, bool invert)
	{
		m_data.clear();
		m_data.shrink_to_fit();

		stbi_set_flip_vertically_on_load(static_cast<int>(!invert));
		stbi_uc* image_data = stbi_load_from_memory(buffer, static_cast<int>(size), &m_width, &m_height, &m_channels, 0);
		m_data.resize(m_width * m_height * m_channels);
		std::copy(image_data, image_data + m_data.size(), m_data.data());
		stbi_image_free(image_data);
		m_is_compressed = false;
		return *this;
	}

	Image& Image::load_from_memory(const Buffer& buffer, bool invert)
	{
		return load_from_memory(buffer.data(), buffer.size(), invert);
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

	Image::Image(Color color, const Size2D& size, uint_t channels)
	{
		create(color, size, channels);
	}

	Image::Image(const Size2D& size, uint_t channels)
	{
		create(size, channels);
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

	Image::~Image() {}


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

	bool Image::save(Path path, ImageType type, bool invert)
	{
		path += extension_of_type(type);

		static bool (Engine::Image::* write_methods[])(const Path& f) = {&Image::write_png, &Image::write_jpg, &Image::write_bmp,
		                                                                 &Image::write_tga};

		auto method = write_methods[static_cast<EnumerateType>(type)];
		stbi_flip_vertically_on_write(static_cast<int>(invert));
		return ((*this).*method)(path);
	}

	Image& Image::create(const Size2D& size, uint_t channels, Buffer&& buffer)
	{
		if (static_cast<int_t>(size.x) * static_cast<int_t>(size.y) * static_cast<int_t>(channels) !=
		    static_cast<int_t>(buffer.size()))
		{
			throw EngineException("Image: Invalid buffer size");
		}

		m_channels = channels;
		m_width    = static_cast<int_t>(size.x);
		m_height   = static_cast<int_t>(size.y);
		m_data     = std::move(buffer);

		return *this;
	}

	Image& Image::create(const Size2D& size, uint_t channels, const Buffer& buffer)
	{
		Buffer copied_buffer = buffer;
		create(size, channels, std::move(copied_buffer));
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

	Image& Image::create(Color color, const Size2D& size, uint_t channels)
	{
		create(size, channels);

		for (int_t x = 0; x < m_width; ++x)
		{
			for (int_t y = 0; y < m_height; ++y)
			{
				byte* data = pixel_at(x, y);

				for (int_t component = 0; component < m_channels; ++component)
				{
					data[component] = color[component];
				}
			}
		}
		return *this;
	}

	Image& Image::create(const Size2D& size, uint_t channels)
	{
		create_interface(size, channels);
		size_t count = m_width * m_height * m_channels;
		m_data.resize(count);
		return *this;
	}

	Image& Image::create_interface(const Size2D& size, uint_t channels)
	{
		m_channels = glm::min<uint_t>(channels, 4);
		m_width    = static_cast<int_t>(glm::abs(size.x));
		m_height   = static_cast<int_t>(glm::abs(size.y));
		m_data.clear();

		return *this;
	}

	byte* Image::pixel_at(uint_t x, uint_t y)
	{
		if (x > static_cast<uint_t>(m_width) || y > static_cast<uint_t>(m_height))
			throw EngineException("Image: Invalid UV");

		uint_t index = y * (m_width * m_channels) + x * m_channels;
		return m_data.data() + index;
	}

	const byte* Image::pixel_at(uint_t x, uint_t y) const
	{
		return const_cast<Image*>(this)->pixel_at(x, y);
	}

	bool Image::is_compressed() const
	{
		return m_is_compressed;
	}

	static void dxt_texture_compress(uint8_t const* const data, Buffer& comp_data, int_t width, int_t height, int_t ncolors)
	{
		assert(width > 0 && height > 0);
		assert(ncolors == 3 || ncolors == 4);
		assert(data != nullptr);
		// RGB=DXT1/BC1, RGBA=DXT5/BC3
		bool const has_alpha  = ncolors == 4;
		const uint_t block_sz = has_alpha ? 16 : 8;
		const uint_t x_blocks = (width + 3) / 4;
		const uint_t y_blocks = (height + 3) / 4;

		comp_data.resize(x_blocks * y_blocks * block_sz);

		for (int_t y = 0; y < height; y += 4)
		{
			uint8_t block[4 * 4 * 4] = {};

			for (int_t x = 0; x < width; x += 4)
			{
				for (int_t yy = 0; yy < 4; ++yy)
				{
					for (int_t xx = 0; xx < 4; ++xx)
					{
						const uint_t bix = 4 * (4 * yy + xx);
						const uint_t dix = ncolors * (width * glm::min(y + yy, height - 1) + glm::min(x + xx, width - 1));
						for (int_t c = 0; c < ncolors; ++c)
						{
							block[bix + c] = data[dix + c];
						}

						if (!has_alpha)
						{
							block[bix + 3] = 255;
						}
					}
				}

				unsigned const comp_offset(((y / 4) * x_blocks + (x / 4)) * block_sz);
				assert(comp_offset < comp_data.size());
				stb_compress_dxt_block(&comp_data[comp_offset], block, has_alpha, /*STB_DXT_NORMAL*/ STB_DXT_HIGHQUAL);
			}
		}
	}

	void Image::compress()
	{
		if (is_compressed())
		{
			error_log("Image", "Cannot recompress image, because image is already compressed!");
			return;
		}

		if (m_channels == 3 || m_channels == 4)
		{
			Buffer compressed_data;
			dxt_texture_compress(m_data.data(), compressed_data, m_width, m_height, m_channels);
			m_data = compressed_data;
		}

		m_is_compressed = true;
	}

	ColorFormat Image::format() const
	{
		if (is_compressed())
		{
			if (m_channels == 3)
			{
				return ColorFormat::BC1;
			}

			if (m_channels == 4)
			{
				return ColorFormat::BC3;
			}
		}

		if (m_channels == 1)
		{
			return ColorFormat::R8;
		}

		if (m_channels == 4)
		{
			return ColorFormat::R8G8B8A8;
		}

		return ColorFormat::Undefined;
	}

	bool Image::serialize(Archive& archive)
	{
		//        if (archive.is_saving() && m_data.empty())
		//        {
		//            error_log("Image", "Failed to serialize image. Data is empty!");
		//            return false;
		//        }

		archive.serialize(m_width, m_height, m_channels, m_is_compressed);

		if (!archive)
		{
			error_log("Image", "Failed to serialize image header!");
			return false;
		}

		archive.serialize(m_data);
		return static_cast<bool>(archive);
	}
}// namespace Engine
