#include <Core/engine_types.hpp>

namespace Engine
{
	class Image;

	struct ENGINE_EXPORT FontConfig {
		Size2D image_size;
		ByteColor3 color;
		UIntVector2D font_size;
		bool dynamic_size;

		FontConfig();
	};

	class ENGINE_EXPORT Font
	{
		void* m_font = nullptr;

	public:
		bool load(const Path& path);
		bool load(const Buffer& buffer);
		bool load(const byte* buffer, size_t size);
		Font& close();
		Size2D calc_text_size(const StringView& text, UIntVector2D font_size) const;
		Image render(const StringView& text, const FontConfig* config = nullptr) const;
		bool is_valid() const;

		~Font();
	};
}// namespace Engine
