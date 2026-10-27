#include <Core/reflection/function.hpp>
#include <Core/reflection/function_set.hpp>

namespace Trinex::Refl
{
	FunctionSet::FunctionSet(Name name, Array<Ref<Function>> functions, Object* owner)
	    : Object(name, 0, owner), m_functions(etl::move(functions))
	{}

	Function* FunctionSet::resolve(Type* const* args, usize count) const
	{
		for (const Ref<Function>& function : m_functions)
		{
			if (function == nullptr)
				continue;

			usize argument_index = 0;
			bool matches         = true;

			for (const Ref<Property>& parameter : function->args())
			{
				if (argument_index == count || parameter->type() != args[argument_index++])
				{
					matches = false;
					break;
				}
			}

			if (matches && argument_index == count)
				return function.value();
		}

		return nullptr;
	}
} // namespace Trinex::Refl
