#include <Core/class.hpp>
#include <Core/etl/singletone.hpp>


namespace Engine
{

    void SingletoneBase::allocate_instance(const Class* class_instance)
    {
        class_instance->create_object();
    }

    void SingletoneBase::begin_destroy_instance(const Class* class_instance)
    {
        Object* object = class_instance->singletone_instance();
        if(object)
        {
            delete object;
        }
    }

    Object* SingletoneBase::extract_object_from_class(const Class* class_instance)
    {
        return class_instance->singletone_instance();
    }
}// namespace Engine
