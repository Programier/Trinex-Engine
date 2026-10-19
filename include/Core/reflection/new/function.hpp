#pragma once

#include <Core/etl/array.hpp>
#include <Core/reflection/new/property.hpp>

namespace Trinex::Refl::New
{
	class trinex_api Function : public Object
	{
	public:
		enum Flags : u64
		{
			IsStatic      = 1 << 0,
			IsConst       = 1 << 1,
			IsVirtual     = 1 << 2,
			IsFinal       = 1 << 3,
			IsPureVirtual = 1 << 4,
			IsConstructor = 1 << 5,
			IsDestructor  = 1 << 6,
			IsOperator    = 1 << 7,
			IsVariadic    = 1 << 8,
		};

	private:
		Ref<Property> m_result;
		Array<Ref<Property>> m_args;
		u64 m_size;
		u64 m_alignment;

	public:
		Function(Name name, Property* result = nullptr, Array<Ref<Property>> args = {}, u64 flags = 0, Object* owner = nullptr);

		inline Property* result() const { return m_result.value(); }
		inline const Array<Ref<Property>>& args() const { return m_args; }

		inline u64 size() const { return m_size; }
		inline u64 alignment() const { return m_alignment; }

		inline bool is_static() const { return has_flags(IsStatic); }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_virtual() const { return has_flags(IsVirtual); }
		inline bool is_final() const { return has_flags(IsFinal); }
		inline bool is_pure_virtual() const { return has_flags(IsPureVirtual); }
		inline bool is_constructor() const { return has_flags(IsConstructor); }
		inline bool is_destructor() const { return has_flags(IsDestructor); }
		inline bool is_pperator() const { return has_flags(IsOperator); }
		inline bool is_variadic() const { return has_flags(IsVariadic); }

		virtual void invoke(void* context) const = 0;
	};
}// namespace Trinex::Refl::New
