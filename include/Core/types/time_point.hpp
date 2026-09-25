#pragma once
#include <Core/etl/string.hpp>

namespace Trinex
{
	class Archive;

	class ENGINE_EXPORT TimePoint final
	{
	public:
		static inline constexpr u64 invalid_value = ~static_cast<u64>(0);

		struct Hash {
			usize operator()(const TimePoint& TimePoint) const noexcept;
		};

	private:
		u64 m_time_point;

	public:
		TimePoint(u64 TimePoint = invalid_value);
		TimePoint(const TimePoint&) = default;
		TimePoint(TimePoint&&)      = default;
		explicit TimePoint(StringView value);
		explicit TimePoint(const String& value);
		explicit TimePoint(const char* value);

		TimePoint& operator=(const TimePoint&) = default;
		TimePoint& operator=(TimePoint&&)      = default;
		TimePoint& operator=(u64 TimePoint);
		TimePoint& operator=(StringView value);
		TimePoint& operator=(const String& value);
		TimePoint& operator=(const char* value);

		static TimePoint now();
		static TimePoint parse(StringView value);
		static bool try_parse(const String& value, TimePoint& out);
		static bool try_parse(const char* value, TimePoint& out);
		static bool try_parse(StringView value, TimePoint& out);

		String to_string() const;
		const TimePoint& to_string(String& out) const;

		bool serialize(Archive& ar);

		inline bool is_valid() const { return m_time_point != invalid_value; }
		inline TimePoint& clear() { trinex_this_return(m_time_point = invalid_value); }
		inline u64 value() const { return m_time_point; }

		inline operator u64() const { return m_time_point; }
		inline bool operator==(const TimePoint& other) const { return m_time_point == other.m_time_point; }
		inline bool operator!=(const TimePoint& other) const { return m_time_point != other.m_time_point; }
		inline bool operator<(const TimePoint& other) const { return m_time_point < other.m_time_point; }
		inline bool operator<=(const TimePoint& other) const { return m_time_point <= other.m_time_point; }
		inline bool operator>(const TimePoint& other) const { return m_time_point > other.m_time_point; }
		inline bool operator>=(const TimePoint& other) const { return m_time_point >= other.m_time_point; }
		inline explicit operator bool() const { return is_valid(); }
	};
}// namespace Trinex

namespace std
{
	template<>
	struct hash<Trinex::TimePoint> {
		size_t operator()(const Trinex::TimePoint& TimePoint) const noexcept
		{
			static Trinex::TimePoint::Hash h;
			return h(TimePoint);
		}
	};
}// namespace std
