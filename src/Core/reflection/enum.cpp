#include <Core/reflection/enum.hpp>

namespace Trinex::Refl
{
	namespace Native
	{
		trinex_api usize parse_enum(const Type* type, void* self, StringView value)
		{
			const Enum* instance = type ? type->cast<Enum>() : nullptr;

			if (instance == nullptr || self == nullptr || instance->underlying() == nullptr)
				return 0;

			auto skip_whitespace = [&value](usize& position) {
				while (position < value.size()
				       && (value[position] == ' ' || value[position] == '\t' || value[position] == '\r'
				           || value[position] == '\n'))
					++position;
			};

			auto find_value = [instance](StringView name) -> const EnumValue* {
				for (const EnumValue& entry : instance->values())
					if (entry.name == name)
						return &entry;

				return nullptr;
			};

			usize position = 0;
			skip_whitespace(position);
			const usize begin = position;
			i64 result        = 0;
			bool has_value    = false;

			for (;;)
			{
				const usize token_begin = position;
				while (position < value.size() && value[position] != '|' && value[position] != ' ' && value[position] != '\t'
				       && value[position] != '\r' && value[position] != '\n')
					++position;

				if (token_begin == position)
					return 0;

				const EnumValue* entry = find_value(value.substr(token_begin, position - token_begin));
				if (entry == nullptr || (!instance->is_bit_flags() && has_value))
				{
					if (has_value)
						return 0;

					const usize parsed = instance->underlying()->parse(self, value.substr(begin));
					return parsed ? begin + parsed : 0;
				}

				if (instance->is_bit_flags())
					result |= entry->value;
				else
					result = entry->value;
				has_value = true;

				skip_whitespace(position);
				if (position == value.size())
					break;

				if (!instance->is_bit_flags() || value[position] != '|')
					return 0;

				++position;
				skip_whitespace(position);
			}

			String numeric;
			return Strings::format(numeric, result) && instance->underlying()->parse(self, numeric) == numeric.size() ? position
			                                                                                                          : 0;
		}

		trinex_api bool format_enum(const Type* type, const void* self, String& value)
		{
			const Enum* instance = type ? type->cast<Enum>() : nullptr;

			if (instance == nullptr || self == nullptr || instance->underlying() == nullptr)
				return false;

			String numeric;
			if (!instance->underlying()->format(self, numeric))
				return false;

			i64 raw_value = 0;
			if (Strings::parse(numeric, raw_value) != numeric.size())
				return false;

			for (const EnumValue& entry : instance->values())
			{
				if (entry.value == raw_value)
				{
					value += entry.name.to_string();
					return true;
				}
			}

			if (!instance->is_bit_flags() || raw_value == 0)
			{
				value += numeric;
				return true;
			}

			i64 remaining  = raw_value;
			bool has_value = false;
			for (const EnumValue& entry : instance->values())
			{
				if (entry.value == 0 || (remaining & entry.value) != entry.value)
					continue;

				if (has_value)
					value += " | ";
				value += entry.name.to_string();
				remaining &= ~entry.value;
				has_value = true;
			}

			if (has_value && remaining == 0)
				return true;

			value += numeric;
			return true;
		}
	} // namespace Native

	Enum::Enum(Name name, Ref<Type> underlying, Array<EnumValue> values, u64 flags, const VTable* vtable, Object* owner)
	    : Type(name, underlying->size(), underlying->alignment(), flags, vtable, owner), m_underlying(etl::move(underlying)),
	      m_values(etl::move(values))
	{}

	const EnumValue* Enum::find_value(Name name) const
	{
		for (const EnumValue& value : m_values)
		{
			if (value.name == name)
				return &value;
		}

		return nullptr;
	}

} // namespace Trinex::Refl
