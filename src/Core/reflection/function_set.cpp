#include <Core/etl/algorithm.hpp>
#include <Core/reflection/function.hpp>
#include <Core/reflection/function_set.hpp>

namespace Trinex::Refl
{
	FunctionSet::FunctionSet(Name name, Object* owner) : Object(name, 0)
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	FunctionSet::FunctionSet(Name name, Array<Ref<Function>> functions, Object* owner)
	    : Object(name, 0), m_functions(etl::move(functions))
	{
		if (owner)
		{
			Object::owner(owner);
		}
	}

	void FunctionSet::invoke_function(Function* function, void* context)
	{
		function->invoke(context);
	}

	bool FunctionSet::register_child(Object* object)
	{
		if (Function* function = cast<Function>(object))
		{
			for (const Ref<Function>& registered : m_functions)
			{
				if (registered == nullptr || registered->args().size() != function->args().size())
					continue;

				bool has_same_arguments = true;

				for (usize index = 0; index < function->args().size(); ++index)
				{
					if (registered->args()[index]->type() != function->args()[index]->type())
					{
						has_same_arguments = false;
						break;
					}
				}

				if (has_same_arguments)
					return false;
			}

			m_functions.emplace_back(retain_ref(function));
			return true;
		}

		return Object::register_child(object);
	}

	bool FunctionSet::unregister_child(Object* object)
	{
		if (Function* function = cast<Function>(object))
		{
			auto it = etl::find(m_functions.begin(), m_functions.end(), function);

			if (it != m_functions.end())
			{
				m_functions.erase_unordered(it);
			}
			return true;
		}

		return Object::unregister_child(object);
	}

	Function* FunctionSet::resolve(Type* const* args, usize count, const Type* result) const
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
