#pragma once
#include <Core/etl/string.hpp>

namespace Trinex
{
	class Archive;

	class ENGINE_EXPORT Timestamp final
	{
	public:
		static inline constexpr u64 invalid_value = ~static_cast<u64>(0);

		struct Hash {
			usize operator()(const Timestamp& timestamp) const noexcept;
		};

	private:
		u64 m_timestamp;

	public:
		Timestamp(u64 timestamp = invalid_value);
		Timestamp(const Timestamp&) = default;
		Timestamp(Timestamp&&)      = default;
		explicit Timestamp(StringView value);
		explicit Timestamp(const String& value);
		explicit Timestamp(const char* value);

		Timestamp& operator=(const Timestamp&) = default;
		Timestamp& operator=(Timestamp&&)      = default;
		Timestamp& operator=(u64 timestamp);
		Timestamp& operator=(StringView value);
		Timestamp& operator=(const String& value);
		Timestamp& operator=(const char* value);

		static Timestamp now();
		static Timestamp parse(StringView value);
		static bool try_parse(const String& value, Timestamp& out);
		static bool try_parse(const char* value, Timestamp& out);
		static bool try_parse(StringView value, Timestamp& out);

		String to_string() const;
		const Timestamp& to_string(String& out) const;

		bool serialize(Archive& ar);

		inline bool is_valid() const { return m_timestamp != invalid_value; }
		inline Timestamp& clear() { trinex_this_return(m_timestamp = invalid_value); }
		inline u64 value() const { return m_timestamp; }

		inline operator u64() const { return m_timestamp; }
		inline bool operator==(const Timestamp& other) const { return m_timestamp == other.m_timestamp; }
		inline bool operator!=(const Timestamp& other) const { return m_timestamp != other.m_timestamp; }
		inline bool operator<(const Timestamp& other) const { return m_timestamp < other.m_timestamp; }
		inline bool operator<=(const Timestamp& other) const { return m_timestamp <= other.m_timestamp; }
		inline bool operator>(const Timestamp& other) const { return m_timestamp > other.m_timestamp; }
		inline bool operator>=(const Timestamp& other) const { return m_timestamp >= other.m_timestamp; }
		inline explicit operator bool() const { return is_valid(); }
	};
}// namespace Trinex

namespace std
{
	template<>
	struct hash<Trinex::Timestamp> {
		size_t operator()(const Trinex::Timestamp& timestamp) const noexcept
		{
			static Trinex::Timestamp::Hash h;
			return h(timestamp);
		}
	};
}// namespace std
