#include <Core/etl/singletone.hpp>
#include <Core/reflection/class.hpp>


namespace Engine
{
	void SingletoneBase::register_singletone(const Refl::Class* class_instance, Object* object)
	{
		class_instance->m_singletone_object = object;
	}

	void SingletoneBase::unlink_instance(const Refl::Class* class_instance)
	{
		class_instance->m_singletone_object = nullptr;
	}

	Object* SingletoneBase::extract_object_from_class(const Refl::Class* class_instance)
	{
		return class_instance->singletone_instance();
	}
}// namespace Engine
