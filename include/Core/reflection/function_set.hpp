#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/invoker.hpp>
#include <Core/reflection/object.hpp>

#include <Core/reflection/function_set.generated.hpp>

namespace Trinex::Refl
{
	class Function;

	trinex_reflect()
	class trinex_api FunctionSet : public Object
	{
		trinex_reflection_body();

	private:
		Array<Ref<Function>> m_functions;

	protected:
		bool register_child(Object* object) override;
		bool unregister_child(Object* object) override;

		static void invoke_function(Function* function, void* context);

	public:
		using Object::resolve;

		FunctionSet(Name name, Object* owner = nullptr);
		FunctionSet(Name name, Array<Ref<Function>> functions = {}, Object* owner = nullptr);

		Function* resolve(Type* const* args, usize count, const Type* result = nullptr) const;

		Class* reflection() const override { return class_of<This>(); }

		inline const Array<Ref<Function>>& functions() const { return m_functions; }
		inline bool empty() const { return m_functions.empty(); }
		inline usize size() const { return m_functions.size(); }

		template<typename... Args>
		Function* resolve()
		{
			if constexpr (sizeof...(Args))
			{
				Type* args[] = {type_of<Args>()...};
				return resolve(args, sizeof...(Args));
			}
			else
			{
				return resolve(nullptr, 0);
			}
		}

		template<typename Ret, typename... Args>
		Ret invoke(Args... args) const
		{
			Function* func = resolve<Args...>();
			trinex_verify_msg(func, "Failed to resolve function");

			etl::InvokeContext<Ret, Args...> context(static_cast<Args&&>(args)...);
			invoke_function(func, context);

			if constexpr (etl::InvokeArg<Ret>::has_value)
			{
				return context.result();
			}
		}
	};
} // namespace Trinex::Refl
