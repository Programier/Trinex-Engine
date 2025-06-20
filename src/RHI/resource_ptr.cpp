#include <RHI/resource_ptr.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	void RHIResourcePtrBase::release(void* object)
	{
		RHI_Object::static_release(static_cast<RHI_Object*>(object));
	}

}// namespace Engine
