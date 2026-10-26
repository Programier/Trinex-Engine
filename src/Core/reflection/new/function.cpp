#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	static void append_type(Property* prop, u32& size, u32& alignment)
	{
		if (prop == nullptr)
		{
			return;
		}

		const Type* type = prop->type();

		if (type == nullptr)
		{
			return;
		}

		alignment = Math::max(alignment, type->alignment());
		size      = align_up(size, type->alignment()) + type->size();
	}

	Function::Function(Name name, Invoker invoker, Ref<Property> result, Array<Ref<Property>> args, u64 flags, Object* owner)
	    : Object(name, flags, owner), m_invoker(invoker), m_result(etl::move(result)), m_args(etl::move(args)), m_size(0),
	      m_alignment(0)
	{
		trinex_verify(m_invoker);

		for (const auto& arg : m_args)
		{
			append_type(arg.value(), m_size, m_alignment);
		}

		append_type(m_result.value(), m_size, m_alignment);
	}

	NativeFunction::NativeFunction(Name name, Address address, Invoker invoker, Ref<Property> result, Array<Ref<Property>> args,
	                               u64 flags, Object* owner)
	    : Function(name, invoker, etl::move(result), etl::move(args), flags, owner), m_address(address)
	{
		trinex_verify(m_address);
	}
}// namespace Trinex::Refl::New
