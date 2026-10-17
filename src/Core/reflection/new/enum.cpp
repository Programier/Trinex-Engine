#include <Core/reflection/new/enum.hpp>

namespace Trinex::Refl::New
{
	Enum::Enum(Name name, u32 size, u32 alignment, Vector<EnumValue> values, u64 flags, const VTable* vtable)
		: Type(name, size, alignment, flags, vtable), m_values(etl::move(values))
	{}

	ObjectKind Enum::kind() const
	{
		return ObjectKind::Enum;
	}

	const EnumValue* Enum::find_value(Name name) const
	{
		for (const EnumValue& value : m_values)
		{
			if (value.name == name)
				return &value;
		}

		return nullptr;
	}
}// namespace Trinex::Refl::New
