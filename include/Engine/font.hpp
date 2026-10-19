#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/array.hpp>
#include <Core/math/vector.hpp>
#include <Core/types/color.hpp>

namespace Trinex
{
	class Image;
	class Path;

	struct trinex_api FontConfig {
		Vector2u image_size;
		Color color;
		Vector2u font_size;
		bool dynamic_size;

		FontConfig();
	};

	class trinex_api Font
	{
		void* m_font = nullptr;

	public:
		bool load(const Path& path);
		bool load(const Buffer& buffer);
		bool load(const u8* buffer, usize size);
		Font& close();
		Vector2u calc_text_size(const StringView& text, Vector2u font_size) const;
		Image render(const StringView& text, const FontConfig* config = nullptr) const;
		bool is_valid() const;

		~Font();
	};
}// namespace Trinex
