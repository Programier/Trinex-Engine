#include <Platform/object.hpp>
#include <Platform/resource_ptr.hpp>

namespace Trinex::Platform
{
	Object::Object(usize init_ref_count) : m_references(init_ref_count) {}

	void Object::static_release_internal(Object* object)
	{
		object->release();
	}

	void Object::add_reference()
	{
		++m_references;
	}

	void Object::release()
	{
		if (m_references > 0)
		{
			--m_references;
		}

		if (m_references == 0)
		{
			destroy();
		}
	}

	usize Object::references() const
	{
		return m_references;
	}

	Object::~Object() {}

	void ResourcePtrBase::release(void* object)
	{
		Object::static_release(static_cast<Object*>(object));
	}
}// namespace Trinex::Platform
