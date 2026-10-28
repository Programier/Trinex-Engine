#pragma once

#include <Core/etl/array.hpp>
#include <Core/etl/invoker.hpp>
#include <Core/reflection/property.hpp>

#include <Core/reflection/function.generated.hpp>

namespace Trinex::Refl
{
	trinex_reflect()
	class trinex_api Function : public Object
	{
		trinex_reflection_body();

	public:
		using Invoker = void (*)(void*, const Function*);

		enum Flags : u64
		{
			IsNative      = 1 << 0,
			IsMethod      = 1 << 1,
			IsConst       = 1 << 2,
			IsVirtual     = 1 << 3,
			IsFinal       = 1 << 4,
			IsConstructor = 1 << 5,
			IsDestructor  = 1 << 6,
			IsOperator    = 1 << 7,
		};

	private:
		Invoker m_invoker;
		Ref<Property> m_result;
		Array<Ref<Property>> m_args;
		u32 m_size;
		u32 m_alignment;

		template<typename... Args>
		bool has_signature() const
		{
			if (m_args.size() != sizeof...(Args))
				return false;

			usize index = 0;
			return ((m_args[index++]->type() == type_of<Args>()) && ...);
		}

	public:
		Function(Name name, Invoker invoker, Ref<Property> result = nullptr, Array<Ref<Property>> args = {}, u64 flags = 0,
		         Object* owner = nullptr);

		Function(Name name, Invoker invoker, Type* result = nullptr, Type** args = nullptr, usize count = 0, u64 flags = 0,
		         Object* owner = nullptr);

		Class* reflection() const override { return class_of<Function>(); }

		inline Property* result() const { return m_result.value(); }
		inline const Array<Ref<Property>>& args() const { return m_args; }
		inline u32 size() const { return m_size; }
		inline u32 alignment() const { return m_alignment; }
		inline Invoker invoker() const { return m_invoker; }
		inline bool is_native() const { return has_flags(IsNative); }
		inline bool is_method() const { return has_flags(IsMethod); }
		inline bool is_const() const { return has_flags(IsConst); }
		inline bool is_virtual() const { return has_flags(IsVirtual); }
		inline bool is_final() const { return has_flags(IsFinal); }
		inline bool is_constructor() const { return has_flags(IsConstructor); }
		inline bool is_destructor() const { return has_flags(IsDestructor); }
		inline bool is_operator() const { return has_flags(IsOperator); }
		inline void invoke(void* context) const { m_invoker(context, this); }

		template<typename Ret, typename... Args>
		Ret invoke(Args... args) const
		{
			trinex_verify(has_signature<Args...>());

			if constexpr (!std::is_void_v<Ret>)
				trinex_verify(m_result && m_result->type() == type_of<Ret>());

			etl::InvokeContext<Ret, Args...> context(static_cast<Args&&>(args)...);
			m_invoker(context, this);

			if constexpr (etl::InvokeArg<Ret>::has_value)
			{
				return context.result();
			}
		}
	};
} // namespace Trinex::Refl
