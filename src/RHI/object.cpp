#include <Core/threading.hpp>
#include <RHI/object.hpp>

namespace Engine
{
	RHIObject::RHIObject(usize init_ref_count) : m_references(init_ref_count) {}

	void RHIObject::static_release_internal(RHIObject* object)
	{
		object->release();
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

	usize RHIObject::references() const
	{
		return m_references;
	}

	RHIObject::~RHIObject() {}
}// namespace Engine
