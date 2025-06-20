#include <Core/exception.hpp>
#include <Core/threading.hpp>
#include <RHI/rhi.hpp>

namespace Engine
{
	ENGINE_EXPORT RHI* rhi = nullptr;

	RHI_Object::RHI_Object(size_t init_ref_count) : m_references(init_ref_count) {}

	void RHI_Object::static_release_internal(RHI_Object* object)
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

	void RHI_Object::add_reference()
	{
		++m_references;
	}

	void RHI_Object::release()
	{
		if (m_references > 0)
			--m_references;

		if (m_references == 0)
		{
			destroy();
		}
	}

	size_t RHI_Object::references() const
	{
		return m_references;
	}

	RHI_Object::~RHI_Object() {}
}// namespace Engine
