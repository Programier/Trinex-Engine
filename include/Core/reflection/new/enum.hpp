#pragma once

#include <Core/etl/array.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	struct EnumValue {
		Name name;
		i64 value;
	};

	class trinex_api Enum final : public Type
	{
	private:
		Array<EnumValue> m_values;

	public:
		enum Flags : u64
		{
			IsBitFlags = 1ull << 8,
			IsScoped   = 1ull << 9,
		};

		Enum(Name name, u32 size, u32 alignment, Array<EnumValue> values = {}, u64 flags = 0, const VTable* vtable = nullptr);

		const EnumValue* find_value(Name name) const;
		inline const Array<EnumValue>& values() const { return m_values; }
		inline bool is_bit_flags() const { return has_flags(IsBitFlags); }
		inline bool is_scoped() const { return has_flags(IsScoped); }
	};
}// namespace Trinex::Refl::New
