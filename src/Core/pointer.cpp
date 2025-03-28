#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/exception.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>

namespace Engine
{
	PointerBase::PointerBase() = default;


	static FORCE_INLINE bool can_update_reference()
	{
		return engine_instance && !engine_instance->is_shuting_down();
	}

	PointerBase& PointerBase::add_reference(Object* object)
	{
		if (object && can_update_reference())
		{
			++object->m_references;
		}
		return *this;
	}

	PointerBase& PointerBase::remove_reference(Object* object)
	{
		if (object && can_update_reference())
		{
			--object->m_references;
		}

		return *this;
	}

	bool PointerBase::serialize(class Archive& ar, Object*& object, bool is_reference)
	{
		if (is_reference)
		{
			ar.serialize_reference(object);
		}
		else
		{
			trinex_always_check(object, "Cannot serialize nullptr object!");
			object->serialize(ar);
		}

		return ar;
	}
}// namespace Engine
