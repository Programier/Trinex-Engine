#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
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
			u64 hash;
			u32 next = 0xFFFFFFFF;
		};

		struct HashFunction {
			inline u64 operator()(const Name& name) const { return name.m_id; }
		};

		struct Less {
			inline bool operator()(const Name& x, const Name& y) const
			{
				return std::less<String>()(x.to_string(), y.to_string());
			}
		};

		static ENGINE_EXPORT Name none;

	private:
		u32 m_id;
		Name& assign(const StringView& view);

	public:
		Name() : m_id(0xFFFFFFFF) {}
		Name(const Name&)            = default;
		Name(Name&&)                 = default;
		Name& operator=(const Name&) = default;
		Name& operator=(Name&&)      = default;

		Name(const char* name) : Name(StringView(name)) {}
		Name(const char* name, usize len) : Name(StringView(name, len)) {}
		Name(const String& name) : Name(StringView(name)) {}
		Name(StringView name) { assign(name); }

		inline Name& operator=(const char* name) { return assign(name); }
		inline Name& operator=(const String& name) { return assign(name); }
		inline Name& operator=(const StringView& name) { return assign(name); }

		u64 hash() const;
		inline bool operator==(const String& name) const { return equals(name); }
		inline bool operator!=(const String& name) const { return !equals(name); }
		inline bool operator==(const StringView& name) const { return equals(name); }
		inline bool operator!=(const StringView& name) const { return !equals(name); }
		inline bool operator==(const char* name) const { return equals(name); }
		inline bool operator!=(const char* name) const { return !equals(name); }

		inline bool equals(const char* name) const { return equals(StringView(name)); }
		inline bool equals(const char* name, usize len) const { return equals(StringView(name, len)); }
		inline bool equals(const Name& name) const { return *this == name; }
		inline bool equals(const String& name) const { return equals(StringView(name)); }
		bool equals(const StringView& name) const;

		const String& to_string() const;
		inline const char* c_str() const { return to_string().c_str(); }
		const Name& to_string(String& out) const;
		inline operator const String&() const { return to_string(); }
		inline operator StringView() const { return StringView(to_string()); }

		inline bool is_valid() const { return m_id != 0xFFFFFFFF; }
		inline u32 id() const { return m_id; }
		inline usize length() const { return to_string().length(); }

		inline operator bool() const { return is_valid(); }
		inline bool operator==(const Name& name) const { return name.m_id == m_id; }
		inline bool operator!=(const Name& name) const { return name.m_id != m_id; }
		inline bool operator<(const Name& name) const { return m_id < name.m_id; }
		inline bool operator<=(const Name& name) const { return m_id <= name.m_id; }
		inline bool operator>(const Name& name) const { return m_id > name.m_id; }
		inline bool operator>=(const Name& name) const { return m_id >= name.m_id; }

		bool serialize(class Archive& ar);
	};
}// namespace Trinex
