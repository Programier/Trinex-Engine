#include <RHI/gc.hpp>
#include <RHI/object.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	void RHIObject::destroy() noexcept
	{
		if (RHI* rhi = device())
		{
			rhi->gc()->add(this);
		}
		else
		{
			trx_delete this;
		}
	}

	RHIObject::~RHIObject() {}
} // namespace Trinex
