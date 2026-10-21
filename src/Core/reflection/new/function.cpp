#include <Core/reflection/new/function.hpp>
#include <Core/reflection/new/type.hpp>

namespace Trinex::Refl::New
{
	Function::Function(Name name, Ref<Property> result, Array<Ref<Property>> args, u64 flags, Object* owner)
	    : Object(name, flags, owner), m_result(etl::move(result)), m_args(etl::move(args))
	{}
}// namespace Trinex::Refl::New
