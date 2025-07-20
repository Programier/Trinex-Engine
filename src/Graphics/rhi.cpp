#include <Core/exception.hpp>
#include <Core/threading.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	ENGINE_EXPORT RHI* rhi = nullptr;

	RHIObject::RHIObject(size_t init_ref_count) : m_references(init_ref_count) {}

	void RHIObject::static_release_internal(RHIObject* object)
	{
		if (is_in_render_thread())
		{
			object->release();
		}
		else
		{
			render_thread()->call([object]() { object->release(); });
		}
	}

	void RHIObject::add_reference()
	{
		++m_references;
	}

	void RHIObject::release()
	{
		if (m_references > 0)
			--m_references;

		if (m_references == 0)
		{
			destroy();
		}
	}

	size_t RHIObject::references() const
	{
		return m_references;
	}

	RHIObject::~RHIObject() {}
}// namespace Engine
