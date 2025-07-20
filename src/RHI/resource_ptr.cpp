#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	void RHIResourcePtrBase::release(void* object)
	{
		RHIObject::static_release(static_cast<RHIObject*>(object));
	}

}// namespace Engine
