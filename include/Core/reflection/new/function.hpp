#pragma once

#include <Core/etl/array.hpp>
#include <Core/reflection/new/property.hpp>

#include <Core/reflection/new/function.generated.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect() class trinex_api Function : public Object
	{
		trinex_reflection_body();

	public:
		enum Flags : u64
		{
			IsMethod      = 1 << 0,
			IsConst       = 1 << 1,
			IsVirtual     = 1 << 2,
			IsFinal       = 1 << 3,
			IsConstructor = 1 << 4,
			IsDestructor  = 1 << 5,
			IsOperator    = 1 << 6,
		};

	private:
		Ref<Property> m_result;
		Array<Ref<Property>> m_args;
		u32 m_size;
		u32 m_alignment;

	public:
		Function(Name name, Ref<Property> result = nullptr, Array<Ref<Property>> args = {}, u64 flags = 0,
		         Object* owner = nullptr);

		Class* reflection() const override { return class_of<Function>(); }

		inline Property* result() const { return m_result.value(); }
		inline const Array<Ref<Property>>& args() const { return m_args; }

		inline u32 size() const { return m_size; }
		inline u32 alignment() const { return m_alignment; }

		inline bool is_method() const { return has_flags(IsMethod); }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_virtual() const { return has_flags(IsVirtual); }
		inline bool is_final() const { return has_flags(IsFinal); }
		inline bool is_constructor() const { return has_flags(IsConstructor); }
		inline bool is_destructor() const { return has_flags(IsDestructor); }
		inline bool is_operator() const { return has_flags(IsOperator); }

		virtual void invoke(void* context) const = 0;
	};
}// namespace Trinex::Refl::New
