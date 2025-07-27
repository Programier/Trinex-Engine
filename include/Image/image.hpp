#pragma once
#include <Core/enums.hpp>
#include <Core/etl/vector.hpp>
#include <Core/types/color.hpp>

namespace Engine
{
	class Archive;
	class Path;
	struct RHIColorFormat;

	class ENGINE_EXPORT Image
	{
	private:
		Buffer m_data;
		Vector2u m_size;

	private:
		void load_from_memory(const void* buffer, size_t size);

	public:
		Image();
		Image(const Path& path);
		Image(const Vector2u& size, uint_t channels = 4, const void* data = nullptr);
		Image(Color color, const Vector2u& size, uint_t channels = 4);
		Image(const void* buffer, size_t size);

		Image(const Image&);
		Image& operator=(const Image&);
		Image(Image&&);
		Image& operator=(Image&&);

		bool resize(const Vector2u& new_size);
		RHIColorFormat format() const;
		bool save(const Path& path);

		byte* sample(uint_t x, uint_t y);
		const byte* sample(uint_t x, uint_t y) const;
		~Image();

		bool serialize(Archive& archive);

		inline byte* data() { return m_data.data(); }
		inline const byte* data() const { return m_data.data(); }
		inline const Buffer& buffer() const { return m_data; }
		inline bool is_empty() const { return m_data.empty() || m_size.x == 0 || m_size.y == 0; }
		inline Vector2u size() const { return m_size; }
		inline uint_t width() const { return m_size.x; }
		inline uint_t height() const { return m_size.y; }
		inline uint_t channels() const { return m_data.size() / (width() * height()); }
	};
}// namespace Engine
