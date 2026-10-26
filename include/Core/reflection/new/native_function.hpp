#pragma once

#include <Core/etl/array.hpp>
#include <Core/etl/invoker.hpp>
#include <Core/reflection/new/function.hpp>

#include <Core/reflection/new/native_function.generated.hpp>

namespace Trinex::Refl::New
{
	trinex_reflect()
	class trinex_api NativeFunction : public Function
	{
		trinex_reflection_body();

	public:
		using Address = void (*)();

	private:
		Address m_address;

	public:
		NativeFunction(Name name, Address address, Invoker invoker, Ref<Property> result = nullptr,
		               Array<Ref<Property>> args = {}, u64 flags = 0, Object* owner = nullptr);

		NativeFunction(Name name, Address address, Invoker invoker, Type* result = nullptr, Type** args = nullptr,
		               usize count = 0, u64 flags = 0, Object* owner = nullptr);

		Class* reflection() const override { return class_of<NativeFunction>(); }

		inline Address address() const { return m_address; }

		template<typename Ret, typename... Args>
		static Ref<NativeFunction> create(StringView name, Ret (*func)(Args...), u64 flags = 0, Object* owner = nullptr)
		{
			constexpr auto resolver = +[](void*, const Function* function) {
				const NativeFunction* native = static_cast<const NativeFunction*>(function);
				return reinterpret_cast<Ret (*)(Args...)>(native->m_address);
			};

			auto address = reinterpret_cast<Address>(func);
			auto invoker = etl::dynamic_invoker<resolver, const Function*>;

			if constexpr (sizeof...(Args) == 0)
			{
				return Ref<NativeFunction>::make(name, address, invoker, type_of<Ret>(), nullptr, 0, flags, owner);
			}
			else
			{
				Type* types[] = {type_of<Args>()...};
				return Ref<NativeFunction>::make(name, address, invoker, type_of<Ret>(), types, sizeof...(Args), flags, owner);
			}
		}
	};
} // namespace Trinex::Refl::New
