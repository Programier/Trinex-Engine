#include <Core/archive.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/time_point.hpp>
#include <ScriptEngine/script_binding.hpp>
#include <chrono>

namespace Trinex
{
	namespace
	{
		using Clock    = std::chrono::steady_clock;
		using Duration = std::chrono::nanoseconds;

		static auto s_start_point = Clock::now();

		static bool TimePoint_equals_string(const TimePoint& self, StringView value)
		{
			TimePoint TimePoint;
			return TimePoint::try_parse(value, TimePoint) && self == TimePoint;
		}

		static bool TimePoint_equals_string_obj(const TimePoint& self, const String& value)
		{
			return TimePoint_equals_string(self, StringView(value));
		}
	}// namespace

	usize TimePoint::Hash::operator()(const TimePoint& TimePoint) const noexcept
	{
		return std::hash<u64>()(TimePoint.value());
	}

	TimePoint::TimePoint(u64 TimePoint) : m_time_point(TimePoint) {}

	TimePoint::TimePoint(StringView value) : TimePoint()
	{
		try_parse(value, *this);
	}

	TimePoint::TimePoint(const String& value) : TimePoint(StringView(value)) {}

	TimePoint::TimePoint(const char* value) : TimePoint(value ? StringView(value) : StringView()) {}

	TimePoint& TimePoint::operator=(u64 TimePoint)
	{
		m_time_point = TimePoint;
		return *this;
	}

	TimePoint& TimePoint::operator=(StringView value)
	{
		try_parse(value, *this);
		return *this;
	}

	TimePoint& TimePoint::operator=(const String& value)
	{
		return operator=(StringView(value));
	}

	TimePoint& TimePoint::operator=(const char* value)
	{
		return operator=(value ? StringView(value) : StringView());
	}

	TimePoint TimePoint::now()
	{
		return TimePoint(std::chrono::duration_cast<Duration>(Clock::now() - s_start_point).count());
	}

	TimePoint TimePoint::parse(StringView value)
	{
		TimePoint TimePoint;
		try_parse(value, TimePoint);
		return TimePoint;
	}

	bool TimePoint::try_parse(const String& value, TimePoint& out)
	{
		return try_parse(StringView(value), out);
	}

	bool TimePoint::try_parse(const char* value, TimePoint& out)
	{
		return try_parse(value ? StringView(value) : StringView(), out);
	}

	bool TimePoint::try_parse(StringView value, TimePoint& out)
	{
		u64 time_point = 0;

		if (!Strings::unsigned_of(value, time_point))
		{
			out = TimePoint();
			return false;
		}

		out = TimePoint(time_point);
		return true;
	}

	String TimePoint::to_string() const
	{
		return Strings::format("{}", m_time_point);
	}

	const TimePoint& TimePoint::to_string(String& out) const
	{
		out += Strings::format("{}", m_time_point);
		return *this;
	}

	bool TimePoint::serialize(Archive& ar)
	{
		return ar.serialize(m_time_point);
	}

	trinex_on_pre_init({.name = "Trinex::TimePoint", .after = {"Trinex::StringView"}})
	{
		auto flags = ScriptClassFlags::Pod | ScriptClassFlags::AppClassAllInts | ScriptClassFlags::AppClassAlign8 |
		             ScriptClassFlags::AppClassMoreConstructors;
		ScriptBinding::Class registrar =
		        ScriptBinding::Class::create("Trinex::TimePoint", ScriptBinding::value_type<TimePoint>(flags));

		registrar.behaviour(ScriptClassBehave::Construct, "void f()", ScriptBinding::Helpers::constructor<TimePoint>,
		                    ScriptCallConv::CDeclObjFirst);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(uint64)", ScriptBinding::Helpers::constructor<TimePoint, u64>,
		                    ScriptCallConv::CDeclObjFirst);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(const string&)",
		                    ScriptBinding::Helpers::constructor<TimePoint, const String&>, ScriptCallConv::CDeclObjFirst);
		registrar.behaviour(ScriptClassBehave::Construct, "void f(const StringView&)",
		                    ScriptBinding::Helpers::constructor<TimePoint, const StringView&>, ScriptCallConv::CDeclObjFirst);

		registrar.static_function("TimePoint now()", &TimePoint::now);
		registrar.static_function("TimePoint parse(const StringView&)", &TimePoint::parse);
		registrar.method("bool is_valid() const", &TimePoint::is_valid);
		registrar.method("Trinex::TimePoint& clear()", &TimePoint::clear);
		registrar.method("uint64 value() const", &TimePoint::value);
		registrar.method("string to_string() const", static_cast<String (TimePoint::*)() const>(&TimePoint::to_string));

		registrar.method("Trinex::TimePoint& opAssign(uint64)",
		                 static_cast<TimePoint& (TimePoint::*) (u64)>(&TimePoint::operator=));
		registrar.method("Trinex::TimePoint& opAssign(const StringView&)",
		                 static_cast<TimePoint& (TimePoint::*) (StringView)>(&TimePoint::operator=));
		registrar.method("Trinex::TimePoint& opAssign(const string&)",
		                 static_cast<TimePoint& (TimePoint::*) (const String&)>(&TimePoint::operator=));
		registrar.method("bool opEquals(const TimePoint&) const", &TimePoint::operator==);
		registrar.method("bool opEquals(const StringView&) const", &TimePoint_equals_string);
		registrar.method("bool opEquals(const string&) const", &TimePoint_equals_string_obj);
	}
}// namespace Trinex
