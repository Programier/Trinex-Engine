#include <Core/reflection/native_function.hpp>

namespace Trinex::Refl
{
	NativeFunction::NativeFunction(Name name, Address address, Invoker invoker, Ref<Property> result, Array<Ref<Property>> args,
	                               u64 flags, Object* owner)
	    : Function(name, invoker, etl::move(result), etl::move(args), flags), m_address(address)
	{
		trinex_verify(m_address);

		if (owner)
		{
			Object::owner(owner);
		}
	}

	NativeFunction::NativeFunction(Name name, Address address, Invoker invoker, Type* result, Type** args, usize count, u64 flags,
	                               Object* owner)
	    : Function(name, invoker, result, args, count, flags), m_address(address)
	{
		trinex_verify(m_address);

		if (owner)
		{
			Object::owner(owner);
		}
	}

} // namespace Trinex::Refl
