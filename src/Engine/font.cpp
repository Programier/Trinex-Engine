#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/math/math.hpp>
#include <Engine/font.hpp>
#include <Image/image.hpp>
#include <freetype/freetype.h>


namespace Engine
{
	static FT_Library m_library;

	static PreInitializeController preinitilize_controller([]() {
		if (FT_Init_FreeType(&m_library))
		{
			throw EngineException("Failed to initialize FreeType library!");
		}
	});

	static PostDestroyController destroy_controller([]() {
		if (FT_Done_FreeType(m_library))
		{
			error_log("Font", "Failed to terminate FreeType library!");
		}
	});


	FontConfig::FontConfig() : image_size(512, 512), color(255, 255, 255, 255), font_size({0, 16}), dynamic_size(false) {}

	static inline FT_Face& make_face(void*& m_face)
	{
		return *reinterpret_cast<FT_Face*>(&m_face);
	}

	static inline const FT_Face& make_face(void* const& m_face)
	{
		return *reinterpret_cast<const FT_Face*>(&m_face);
	}

	Font& Font::close()
	{
		if (m_font)
		{
			FT_Face& face = make_face(m_font);
			FT_Done_Face(face);
			face = nullptr;
		}

		return *this;
	}

	bool Font::load(const Path& path)
	{
		FileReader reader(path);

		if (!reader.is_open())
			return false;

		Buffer buffer = reader.read_buffer();
		return load(buffer);
	}

	bool Font::load(const Buffer& buffer)
	{
		return load(buffer.data(), buffer.size());
	}

	bool Font::load(const byte* buffer, size_t size)
	{
		if (m_font)
		{
			close();
		}

		if (FT_New_Memory_Face(m_library, buffer, size, 0, &make_face(m_font)))
		{
			error_log("Font", "Failed to create font face!");
			close();
			return false;
		}

		return true;
	}

	static FontConfig* default_font_config()
	{
		static FontConfig config;
		return &config;
	}

	static void draw_bitmap(Image& image, const FT_GlyphSlot& glyph, uint_t x, uint_t y, const FontConfig* config)
	{
		uint_t image_width  = image.width();
		uint_t image_height = image.height();

		FT_Bitmap bitmap = glyph->bitmap;

		for (uint_t i = 0; i < bitmap.rows; ++i)
		{
			for (uint_t j = 0; j < bitmap.width; ++j)
			{
				uint_t img_x = x + glyph->bitmap_left + j;

				uint_t img_y = (y - ((glyph->metrics.height >> 6) - glyph->bitmap_top)) + (bitmap.rows - i);

				if (img_x >= 0 && img_x < image_width && img_y >= 0 && img_y < image_height)
				{
					unsigned char value = bitmap.buffer[i * bitmap.pitch + j];

					byte* data = image.sample(img_x, img_y);
					data[0]    = config->color.r;
					data[1]    = config->color.g;
					data[2]    = config->color.b;
					data[3]    = value;
				}
			}
		}
	}

	Vector2u Font::calc_text_size(const StringView& text, Vector2u font_size) const
	{
		if (!is_valid() || text.empty())
			return {0, 0};

		const FT_Face& face = make_face(m_font);
		FT_Set_Pixel_Sizes(face, font_size.x, font_size.y);

		uint_t size_x    = 0;
		uint_t size_y    = font_size.y;
		uint_t current_x = 0;

		for (char p : text)
		{
			if (p == '\n')
			{
				size_y += font_size.y;
				current_x = 0;
				continue;
			}

			if (FT_Load_Char(face, p, FT_LOAD_RENDER))
			{
				continue;
			}

			current_x += face->glyph->advance.x >> 6;
			size_x = Math::max(size_x, current_x);
			size_y = Math::max(size_y,
			                   font_size.y + static_cast<uint_t>(face->glyph->metrics.height >> 6) - face->glyph->bitmap_top);
		}

		return {static_cast<float>(size_x), static_cast<float>(size_y)};
	}

	Image Font::render(const StringView& text, const FontConfig* config) const
	{
		if (!is_valid())
			return {};

		if (config == nullptr)
			config = default_font_config();

		const FT_Face& face = make_face(m_font);

		if (!config->dynamic_size)
			FT_Set_Pixel_Sizes(face, config->font_size.x, config->font_size.y);

		Vector2u image_size = config->dynamic_size ? calc_text_size(text, config->font_size) : config->image_size;
		Image image(image_size, 4);

		if (config->font_size.y == 0)
		{
			error_log("FontConfig", "font_size.y cannot be 0!");
			return image;
		}

		uint_t pen_x        = 0;
		uint_t pen_y        = static_cast<uint_t>(image_size.y) - config->font_size.y;
		uint_t image_size_x = image.width();

		for (char p : text)
		{
			if (p == '\n')
			{
				pen_y -= config->font_size.y;
				pen_x = 0;
				continue;
			}

			if (FT_Load_Char(face, p, FT_LOAD_RENDER))
			{
				continue;
			}

			uint_t advance = face->glyph->advance.x >> 6;

			if (pen_x + advance > image_size_x)
			{
				pen_y -= config->font_size.y;
				pen_x = 0;
			}

			draw_bitmap(image, face->glyph, pen_x, pen_y, config);
			pen_x += advance;
		}

		return image;
	}

	bool Font::is_valid() const
	{
		return m_font != nullptr;
	}

	Font::~Font()
	{
		close();
	}
}// namespace Engine
