#include <Core/archive.hpp>
#include <Core/file_manager.hpp>
#include <Core/filesystem/file.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/filesystem/root_filesystem.hpp>
#include <Image/image.hpp>
#include <RHI/enums.hpp>
#include <cstring>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>

namespace Engine
{
	Image::Image() = default;

	Image::Image(const Path& path)
	{
		Buffer buffer = FileReader(path).read_buffer();
		load_from_memory(buffer.data(), buffer.size());
	}

	Image::Image(const Vector2u& size, u32 channels, const void* data) : m_size(size)
	{
		m_data.resize(width() * height() * channels);

		if (data)
			std::memcpy(m_data.data(), data, m_data.size());
	}

	Image::Image(Color color, const Vector2u& size, u32 channels) : m_size(size)
	{
		m_data.resize(width() * height() * channels);

		for (u32 x = 0; x < width(); ++x)
		{
			for (u32 y = 0; y < height(); ++y)
			{
				u8* data = sample(x, y);

				for (u32 component = 0; component < channels; ++component)
				{
					data[component] = color[component];
				}
			}
		}
	}

	Image::Image(const void* data, usize size)
	{
		load_from_memory(data, size);
	}

	Image::Image(const Image& img)
	{
		*this = img;
	}

	Image& Image::operator=(const Image& img)
	{
		if (this == &img)
			return *this;
		m_data = img.m_data;
		m_size = img.m_size;
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

		m_data = std::move(img.m_data);
		m_size = img.m_size;

		img.m_size = {0, 0};
		return *this;
	}

	void Image::load_from_memory(const void* buffer, usize size)
	{
		i32 pixel_channels = 0;
		int w, h;
		stbi_uc* image_data =
		        stbi_load_from_memory(static_cast<const stbi_uc*>(buffer), static_cast<int>(size), &w, &h, &pixel_channels, 0);
		m_size.x = static_cast<u32>(w);
		m_size.y = static_cast<u32>(h);

		if (pixel_channels != 3)
		{
			m_data.resize(m_size.x * m_size.y * pixel_channels);
			std::copy(image_data, image_data + m_data.size(), m_data.data());
			stbi_image_free(image_data);
			return;
		}

		m_data.resize(m_size.x * m_size.y * 4, 255);
		u8* dst = m_data.data();

		for (int x = 0; x < w; ++x)
		{
			for (int y = 0; y < h; ++y)
			{
				(*dst++) = (*image_data++);
				(*dst++) = (*image_data++);
				(*dst++) = (*image_data++);
				++dst;
			}
		}
	}

	bool Image::resize(const Vector2u& size)
	{
		Vector<u8> resized_image(size.x * size.y * channels(), 0);

		stbir_resize_uint8_linear(m_data.data(), width(), height(), width() * channels(), resized_image.data(), size.x, size.y,
		                          size.x * channels(), STBIR_RGBA);
		m_size = size;
		m_data = std::move(resized_image);
		return true;
	}

	static void image_writer_func(void* context, void* data, int size)
	{
		VFS::File* file = static_cast<VFS::File*>(context);
		file->write(static_cast<u8*>(data), size);
	}

	bool Image::save(const Path& path)
	{
		StringView extension = path.extension();
		VFS::File* file      = rootfs()->open(path, FileOpenMode::Out);

		if (file == nullptr)
			return false;

		trinex_defer
		{
			rootfs()->close(file);
		};

		if (extension == ".png")
		{
			return stbi_write_png_to_func(image_writer_func, file, width(), height(), channels(), data(), width() * channels());
		}

		if (extension == ".bmp")
		{
			return stbi_write_bmp_to_func(image_writer_func, file, width(), height(), channels(), data());
		}

		if (extension == ".tga")
		{
			return stbi_write_tga_to_func(image_writer_func, file, width(), height(), channels(), data());
		}

		if (extension == ".hdr")
		{
			return stbi_write_tga_to_func(image_writer_func, file, width(), height(), channels(), data());
		}

		if (extension == ".jpg" || extension == ".jpeg")
		{
			return stbi_write_jpg_to_func(image_writer_func, file, width(), height(), channels(), data(), 100);
		}

		return false;
	}

	u8* Image::sample(u32 x, u32 y)
	{
		trinex_assert_msg(x < m_size.x && y < m_size.y, "Invalid sample coords");

		u32 index = y * (width() * channels()) + x * channels();
		return m_data.data() + index;
	}

	const u8* Image::sample(u32 x, u32 y) const
	{
		return const_cast<Image*>(this)->sample(x, y);
	}

	RHIColorFormat Image::format() const
	{
		switch (channels())
		{
			case 1: return RHIColorFormat::R8;
			case 2: return RHIColorFormat::R8G8;
			case 4: return RHIColorFormat::R8G8B8A8;
			default: return RHIColorFormat::Undefined;
		}
	}

	bool Image::serialize(Archive& archive)
	{
		return archive.serialize(m_size, m_data);
	}
}// namespace Engine
