#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	void RHIResourcePtrBase::release(void* object)
	{
		RHIObject::static_release(static_cast<RHIObject*>(object));
	}

}// namespace Trinex
