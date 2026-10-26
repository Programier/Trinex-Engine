#include <Core/reflection/new/native_function.hpp>

namespace Trinex::Refl::New
{
	NativeFunction::NativeFunction(Name name, Address address, Invoker invoker, Ref<Property> result, Array<Ref<Property>> args,
	                               u64 flags, Object* owner)
	    : Function(name, invoker, etl::move(result), etl::move(args), flags, owner), m_address(address)
	{
		trinex_verify(m_address);
	}

	NativeFunction::NativeFunction(Name name, Address address, Invoker invoker, Type* result, Type** args, usize count, u64 flags,
	                               Object* owner)
	    : Function(name, invoker, result, args, count, flags, owner), m_address(address)
	{
		trinex_verify(m_address);
	}

}// namespace Trinex::Refl::New
