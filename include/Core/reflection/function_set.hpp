#pragma once
#include <Core/etl/array.hpp>
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

	public:
		using Object::resolve;

		FunctionSet(Name name, Array<Ref<Function>> functions = {}, Object* owner = nullptr);

		Function* resolve(Type* const* args, usize count) const;

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
	};
} // namespace Trinex::Refl
