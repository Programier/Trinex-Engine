#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	Function::Function(Name name, const Type* type, Vector<Parameter> args, u64 flags, const VTable* vtable)
	    : Object(name, flags), m_type(type), m_args(etl::move(args)), m_vtable(vtable)
	{}

	Function::InvokeResult Function::invoke(void* context, const Argument* args, usize argument_count, void* return_value) const
	{
		if (m_vtable == nullptr)
			return InvokeResult::MissingVTable;
		if (!is_static() && context == nullptr)
			return InvokeResult::InvalidContext;
		if (m_type != nullptr && return_value == nullptr)
			return InvokeResult::MissingReturnStorage;

		const usize parameter_count = m_args.size();
		const bool is_variadic      = parameter_count != 0 && m_args.back().is_variadic();
		const usize fixed_count     = parameter_count - static_cast<usize>(is_variadic);
		if ((is_variadic && argument_count < fixed_count) || (!is_variadic && argument_count != parameter_count))
			return InvokeResult::InvalidArgumentCount;
		if (argument_count != 0 && args == nullptr)
			return InvokeResult::InvalidArgumentStorage;

		for (usize index = 0; index < argument_count; ++index)
		{
			const Parameter& parameter = m_args[is_variadic && index >= fixed_count ? parameter_count - 1 : index];
			const Argument& argument   = args[index];
			if (argument.data == nullptr)
				return InvokeResult::InvalidArgumentStorage;
			if (parameter.type == nullptr || argument.type != parameter.type)
				return InvokeResult::InvalidArgumentType;
		}

		return m_vtable->invoke(context, args, argument_count, return_value);
	}
}// namespace Trinex::Refl::New
