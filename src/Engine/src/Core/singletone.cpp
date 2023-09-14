#include <Core/class.hpp>
#include <Core/etl/singletone.hpp>


namespace Engine
{
    void SingletoneBase::register_singletone_object(Object* object, const Class* _class)
    {
        if (_class)
        {
            Class* class_instance = const_cast<Class*>(_class);
            //class_instance->_M_singletone_instance = object;
        }
    }
}// namespace Engine
