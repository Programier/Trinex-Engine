#include <RHI/gc.hpp>
#include <RHI/object.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	RHIGarbageGroup* RHIObject::garbage_group() const
	{
		return nullptr;
	}

	void RHIObject::on_reclaimable() {}

	void RHIObject::destroy() noexcept
	{
		if (RHI* rhi = device(); rhi && rhi->gc())
		{
			rhi->gc()->add(this, garbage_group());
		}
		else
		{
			trx_delete this;
		}
	}

	RHIObject::~RHIObject() {}
} // namespace Trinex
