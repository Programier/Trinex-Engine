#include <Core/pointer.hpp>
#include <Core/object.hpp>

namespace Engine
{
    PointerBase::PointerBase() = default;

    PointerBase& PointerBase::add_reference(Object* object)
    {
        if(object)
        {
            ++object->_M_references;
        }
        return *this;
    }

    PointerBase& PointerBase::remove_reference(Object* object)
    {
        if(object)
        {
            --object->_M_references;
        }

        return *this;
    }
}
