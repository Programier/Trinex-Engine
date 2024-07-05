#include <Core/class.hpp>
#include <Core/etl/singletone.hpp>


namespace Engine
{
    void SingletoneBase::register_singletone(const Class* class_instance, Object* object)
    {
        class_instance->m_singletone_object = object;
    }

    void SingletoneBase::unlink_instance(const Class* class_instance)
    {
        class_instance->m_singletone_object = nullptr;
    }

    Object* SingletoneBase::extract_object_from_class(const Class* class_instance)
    {
        return class_instance->singletone_instance();
    }
}// namespace Engine
