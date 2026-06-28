#include <Core/archive.hpp>
#include <Core/string_functions.hpp>
#include <Core/types/timestamp.hpp>
#include <ScriptEngine/registrar.hpp>
#include <chrono>

namespace Trinex
{
	namespace
	{
		using Clock    = std::chrono::steady_clock;
		using Duration = std::chrono::nanoseconds;

		static auto s_start_point = Clock::now();

		static bool timestamp_equals_string(const Timestamp& self, StringView value)
		{
			Timestamp timestamp;
			return Timestamp::try_parse(value, timestamp) && self == timestamp;
		}

		static bool timestamp_equals_string_obj(const Timestamp& self, const String& value)
		{
			return timestamp_equals_string(self, StringView(value));
		}
	}// namespace

	usize Timestamp::Hash::operator()(const Timestamp& timestamp) const noexcept
	{
		return std::hash<u64>()(timestamp.value());
	}

	Timestamp::Timestamp(u64 timestamp) : m_timestamp(timestamp) {}

	Timestamp::Timestamp(StringView value) : Timestamp()
	{
		try_parse(value, *this);
	}

	Timestamp::Timestamp(const String& value) : Timestamp(StringView(value)) {}

	Timestamp::Timestamp(const char* value) : Timestamp(value ? StringView(value) : StringView()) {}

	Timestamp& Timestamp::operator=(u64 timestamp)
	{
		m_timestamp = timestamp;
		return *this;
	}

	Timestamp& Timestamp::operator=(StringView value)
	{
		try_parse(value, *this);
		return *this;
	}

	Timestamp& Timestamp::operator=(const String& value)
	{
		return operator=(StringView(value));
	}

	Timestamp& Timestamp::operator=(const char* value)
	{
		return operator=(value ? StringView(value) : StringView());
	}

	Timestamp Timestamp::now()
	{
		return Timestamp(std::chrono::duration_cast<Duration>(Clock::now() - s_start_point).count());
	}

	Timestamp Timestamp::parse(StringView value)
	{
		Timestamp timestamp;
		try_parse(value, timestamp);
		return timestamp;
	}

	bool Timestamp::try_parse(const String& value, Timestamp& out)
	{
		return try_parse(StringView(value), out);
	}

	bool Timestamp::try_parse(const char* value, Timestamp& out)
	{
		return try_parse(value ? StringView(value) : StringView(), out);
	}

	bool Timestamp::try_parse(StringView value, Timestamp& out)
	{
		u64 timestamp = 0;

		if (!Strings::unsigned_of(value, timestamp))
		{
			out = Timestamp();
			return false;
		}

		out = Timestamp(timestamp);
		return true;
	}

	String Timestamp::to_string() const
	{
		return Strings::format("{}", m_timestamp);
	}

	const Timestamp& Timestamp::to_string(String& out) const
	{
		out += Strings::format("{}", m_timestamp);
		return *this;
	}

	bool Timestamp::serialize(Archive& ar)
	{
		return ar.serialize(m_timestamp);
	}

	trinex_on_pre_init({.name = "Trinex::Timestamp", .after = {"Trinex::StringView"}})
	{
		ScriptClassRegistrar::ValueInfo info;
		info.all_ints          = true;
		info.more_constructors = true;
		info.pod               = true;
		info.has_constructor   = true;
		info.align8            = true;

		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Trinex::Timestamp", sizeof(Timestamp), info);

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Timestamp>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(uint64)", ScriptClassRegistrar::constructor<Timestamp, u64>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const string&)",
		                 ScriptClassRegistrar::constructor<Timestamp, const String&>, ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const StringView&)",
		                 ScriptClassRegistrar::constructor<Timestamp, const StringView&>, ScriptCallConv::CDeclObjFirst);

		registrar.static_function("Timestamp now()", &Timestamp::now);
		registrar.static_function("Timestamp parse(const StringView&)", &Timestamp::parse);
		registrar.method("bool is_valid() const", &Timestamp::is_valid);
		registrar.method("Trinex::Timestamp& clear()", &Timestamp::clear);
		registrar.method("uint64 value() const", &Timestamp::value);
		registrar.method("string to_string() const", static_cast<String (Timestamp::*)() const>(&Timestamp::to_string));

		registrar.method("Trinex::Timestamp& opAssign(uint64)",
		                 static_cast<Timestamp& (Timestamp::*) (u64)>(&Timestamp::operator=));
		registrar.method("Trinex::Timestamp& opAssign(const StringView&)",
		                 static_cast<Timestamp& (Timestamp::*) (StringView)>(&Timestamp::operator=));
		registrar.method("Trinex::Timestamp& opAssign(const string&)",
		                 static_cast<Timestamp& (Timestamp::*) (const String&)>(&Timestamp::operator=));
		registrar.method("bool opEquals(const Timestamp&) const", &Timestamp::operator==);
		registrar.method("bool opEquals(const StringView&) const", &timestamp_equals_string);
		registrar.method("bool opEquals(const string&) const", &timestamp_equals_string_obj);
	}
}// namespace Trinex
