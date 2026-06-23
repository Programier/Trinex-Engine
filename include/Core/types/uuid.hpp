#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>

namespace Trinex
{
	class Archive;

	class ENGINE_EXPORT UUID final
	{
	public:
		static inline constexpr usize byte_count = 16;

		struct Hash {
			usize operator()(const UUID& uuid) const noexcept;
		};

	private:
		u128 m_uuid;

	public:
		UUID(u128 uuid = 0);
		UUID(const UUID&) = default;
		UUID(UUID&&)      = default;
		UUID(u64 high, u64 low);
		explicit UUID(StringView value);
		explicit UUID(const String& value);
		explicit UUID(const char* value);

		UUID& operator=(const UUID&) = default;
		UUID& operator=(UUID&&)      = default;
		UUID& operator=(StringView value);
		UUID& operator=(const String& value);
		UUID& operator=(const char* value);

		static UUID generate();
		static UUID parse(StringView value);
		static bool try_parse(const String& value, UUID& out);
		static bool try_parse(const char* value, UUID& out);
		static bool try_parse(StringView value, UUID& out);

		u8 variant() const;

		String to_string() const;
		const UUID& to_string(String& out) const;

		bool serialize(Archive& ar);

		inline u8 version() const { return static_cast<u8>((data()[6] >> 4) & 0x0F); }
		inline bool is_valid() const { return m_uuid != 0; }
		inline UUID& clear() { trinex_this_return(m_uuid = 0); }

		inline const u8* data() const { return reinterpret_cast<const u8*>(&m_uuid); }
		inline u8* data() { return reinterpret_cast<u8*>(&m_uuid); }

		inline operator u128() const { return m_uuid; }
		inline bool operator==(const UUID& other) const { return m_uuid == other.m_uuid; }
		inline bool operator!=(const UUID& other) const { return m_uuid != other.m_uuid; }
		inline bool operator<(const UUID& other) const { return m_uuid < other.m_uuid; }
		inline bool operator<=(const UUID& other) const { return m_uuid <= other.m_uuid; }
		inline bool operator>(const UUID& other) const { return m_uuid > other.m_uuid; }
		inline bool operator>=(const UUID& other) const { return m_uuid >= other.m_uuid; }
		inline explicit operator bool() const { return is_valid(); }
	};
}// namespace Trinex

namespace std
{
	template<>
	struct hash<Trinex::UUID> {
		size_t operator()(const Trinex::UUID& uuid) const noexcept
		{
			static Trinex::UUID::Hash h;
			return h(uuid);
		}
	};
}// namespace std
