#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Engine
{
	class ENGINE_EXPORT Name
	{
	public:
		static const Name undefined;
		static const Name color;
		static const Name offset;
		static const Name ambient_color;
		static const Name radius;
		static const Name fall_off_exponent;
		static const Name height;
		static const Name spot_angles;
		static const Name intensivity;
		static const Name location;
		static const Name direction;
		static const Name inverse_rotation;
		static const Name scale;
		static const Name out_of_range;
		static const Name model;
		static const Name texture;
		static const Name mask;

	public:
		struct Entry {
			String name;
			uint64_t hash;
		};

		struct HashFunction {
			FORCE_INLINE uint64_t operator()(const Name& name) const { return name.m_index; }
		};

		struct Less {
			FORCE_INLINE bool operator()(const Name& x, const Name& y) const
			{
				return std::less<String>()(x.to_string(), y.to_string());
			}
		};

		static ENGINE_EXPORT Name none;

	private:
		uint32_t m_index;
		Name& init(const StringView& view);

	public:
		Name();
		Name(const Name&);
		Name(Name&&);
		Name& operator=(const Name&);
		Name& operator=(Name&&);

		Name(const char* name);
		Name(const char* name, size_t len);
		Name(const String& name);
		Name(const StringView& name);

		Name& operator=(const char* name);
		Name& operator=(const String& name);
		Name& operator=(const StringView& name);

		static Name find_name(const StringView& name);
		static size_t static_count();

		uint64_t hash() const;
		bool operator==(const StringView& name) const;
		bool operator!=(const StringView& name) const;
		bool operator==(const char* name) const;
		bool operator!=(const char* name) const;
		bool operator==(const String& name) const;
		bool operator!=(const String& name) const;

		bool equals(const String& name) const;
		bool equals(const char* name) const;
		bool equals(const char* name, size_t len) const;
		bool equals(const StringView& name) const;
		bool equals(const Name& name) const;

		const String& to_string() const;
		const char* c_str() const;
		const Name& to_string(String& out) const;
		operator const String&() const;
		operator StringView() const;

		inline bool is_valid() const { return m_index != 0xFFFFFFFF; }
		inline uint32_t index() const { return m_index; }
		inline size_t length() const { return to_string().length(); }

		inline bool operator==(const Name& name) const { return name.m_index == m_index; }
		inline bool operator!=(const Name& name) const { return name.m_index != m_index; }
		inline bool operator<(const Name& name) const { return m_index < name.m_index; }
		inline bool operator<=(const Name& name) const { return m_index <= name.m_index; }
		inline bool operator>(const Name& name) const { return m_index > name.m_index; }
		inline bool operator>=(const Name& name) const { return m_index >= name.m_index; }

		bool serialize(class Archive& ar);
	};
}// namespace Engine
