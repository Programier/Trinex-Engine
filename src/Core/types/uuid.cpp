#include <Core/archive.hpp>
#include <Core/types/uuid.hpp>
#include <Platform/platform.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Trinex
{
	namespace
	{
		static constexpr char hex_digits[] = "0123456789ABCDEF";

		static FORCE_INLINE bool is_uuid_separator(char ch)
		{
			return ch == '-' || ch == '{' || ch == '}';
		}

		static FORCE_INLINE i32 hex_value_of(char ch)
		{
			if (ch >= '0' && ch <= '9')
				return ch - '0';

			if (ch >= 'a' && ch <= 'f')
				return ch - 'a' + 10;

			if (ch >= 'A' && ch <= 'F')
				return ch - 'A' + 10;

			return -1;
		}

		static FORCE_INLINE usize compact_uuid(StringView value, char* out)
		{
			usize count = 0;

			for (char ch : value)
			{
				if (is_uuid_separator(ch))
					continue;

				if (count >= 32)
					return count;

				out[count++] = ch;
			}

			return count;
		}

		static bool uuid_equals_string(const UUID& self, const StringView& value)
		{
			UUID uuid;
			return UUID::try_parse(value, uuid) && self == uuid;
		}

		static bool uuid_equals_string_obj(const UUID& self, const String& value)
		{
			return uuid_equals_string(self, StringView(value));
		}
	}// namespace

	usize UUID::Hash::operator()(const UUID& uuid) const noexcept
	{
		usize result = 1469598103934665603ull;

		for (usize index = 0; index < UUID::byte_count; ++index)
		{
			const u8 byte = uuid.data()[index];
			result ^= byte;
			result *= 1099511628211ull;
		}

		return result;
	}

	UUID::UUID(u128 uuid) : m_uuid(uuid) {}

	UUID::UUID(u64 high, u64 low) : UUID((u128(high) << 64) | u128(low)) {}

	UUID::UUID(StringView value) : UUID()
	{
		try_parse(value, *this);
	}

	UUID::UUID(const String& value) : UUID(StringView(value)) {}

	UUID::UUID(const char* value) : UUID(value ? StringView(value) : StringView()) {}

	UUID& UUID::operator=(StringView value)
	{
		try_parse(value, *this);
		return *this;
	}

	UUID& UUID::operator=(const String& value)
	{
		return operator=(StringView(value));
	}

	UUID& UUID::operator=(const char* value)
	{
		return operator=(value ? StringView(value) : StringView());
	}

	UUID UUID::generate()
	{
		UUID uuid;
		Platform::create_uuid(uuid);
		return uuid;
	}

	UUID UUID::parse(StringView value)
	{
		UUID uuid;
		try_parse(value, uuid);
		return uuid;
	}

	bool UUID::try_parse(const String& value, UUID& out)
	{
		return try_parse(StringView(value), out);
	}

	bool UUID::try_parse(const char* value, UUID& out)
	{
		return try_parse(value ? StringView(value) : StringView(), out);
	}

	bool UUID::try_parse(StringView value, UUID& out)
	{
		char compact[32];
		usize count = compact_uuid(value, compact);

		out = UUID();

		if (count != 32)
			return false;

		for (usize index = 0; index < byte_count; ++index)
		{
			const i32 high = hex_value_of(compact[index * 2]);
			const i32 low  = hex_value_of(compact[index * 2 + 1]);

			if (high < 0 || low < 0)
			{
				out = UUID();
				return false;
			}

			out.data()[index] = static_cast<u8>((high << 4) | low);
		}

		return true;
	}

	u8 UUID::variant() const
	{
		const u8 octet = data()[8];

		if ((octet & 0x80) == 0x00)
			return 0;

		if ((octet & 0xC0) == 0x80)
			return 1;

		if ((octet & 0xE0) == 0xC0)
			return 2;

		return 3;
	}

	String UUID::to_string() const
	{
		String out;
		out.reserve(36);
		to_string(out);
		return out;
	}

	const UUID& UUID::to_string(String& out) const
	{
		out.reserve(out.size() + 36);

		for (usize index = 0; index < byte_count; ++index)
		{
			if (index == 4 || index == 6 || index == 8 || index == 10)
				out.push_back('-');

			const u8 byte = data()[index];
			out.push_back(hex_digits[(byte >> 4) & 0x0F]);
			out.push_back(hex_digits[byte & 0x0F]);
		}

		return *this;
	}

	bool UUID::serialize(Archive& ar)
	{
		return ar.serialize(m_uuid);
	}

	trinex_on_pre_init({.name = "Trinex::UUID", .after = {"Trinex::StringView"}})
	{
		ScriptClassRegistrar::ValueInfo info;
		info.all_ints          = true;
		info.more_constructors = true;
		info.pod               = true;
		info.has_constructor   = true;
		info.align8            = true;

		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class("Trinex::UUID", sizeof(UUID), info);

		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<UUID>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const string&)",
		                 ScriptClassRegistrar::constructor<UUID, const String&>, ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, "void f(const StringView&)",
		                 ScriptClassRegistrar::constructor<UUID, const StringView&>, ScriptCallConv::CDeclObjFirst);

		registrar.static_function("UUID generate()", &UUID::generate);
		registrar.static_function("UUID parse(const StringView&)", &UUID::parse);
		registrar.method("bool is_valid() const", &UUID::is_valid);
		registrar.method("Trinex::UUID& clear()", &UUID::clear);
		registrar.method("uint8 version() const", &UUID::version);
		registrar.method("uint8 variant() const", &UUID::variant);
		registrar.method("string to_string() const", static_cast<String (UUID::*)() const>(&UUID::to_string));

		registrar.method("Trinex::UUID& opAssign(const StringView&)",
		                 static_cast<UUID& (UUID::*) (StringView)>(&UUID::operator=));
		registrar.method("Trinex::UUID& opAssign(const string&)", static_cast<UUID& (UUID::*) (const String&)>(&UUID::operator=));
		registrar.method("bool opEquals(const UUID&) const", &UUID::operator==);
		registrar.method("bool opEquals(const StringView&) const", &uuid_equals_string);
		registrar.method("bool opEquals(const string&) const", &uuid_equals_string_obj);
	}
}// namespace Trinex
