#pragma once

#include <Core/etl/array.hpp>
#include <Core/reflection/type.hpp>

#include <Core/reflection/enum.generated.hpp>

namespace Trinex::Refl
{
	struct EnumValue
	{
		Name name;
		i64 value;
	};

	trinex_reflect()
	class trinex_api Enum : public Type
	{
		trinex_reflection_body();

	public:
		struct VTable : public Type::VTable
		{
		};

		enum Flags : u64
		{
			IsBitFlags = 1ull << 8,
			IsScoped   = 1ull << 9,
		};

	private:
		Ref<Type> m_underlying;
		Array<EnumValue> m_values;

	public:
		Enum(Name name, Ref<Type> underlying, Array<EnumValue> values = {}, u64 flags = 0, const VTable* vtable = nullptr,
		     Object* owner = nullptr);

		Class* reflection() const override { return class_of<Enum>(); }

		const EnumValue* find_value(Name name) const;
		inline const VTable* vtable() const { return static_cast<const VTable*>(Type::vtable()); }
		inline Type* underlying() const { return m_underlying.value(); }
		inline const Array<EnumValue>& values() const { return m_values; }
		inline bool is_bit_flags() const { return has_flags(IsBitFlags); }
		inline bool is_scoped() const { return has_flags(IsScoped); }
	};
} // namespace Trinex::Refl
