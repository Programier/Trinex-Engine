#include <Core/engine.hpp>
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
            ++object->_M_references;
        }
        return *this;
    }

    PointerBase& PointerBase::remove_reference(Object* object)
    {
        if (object && can_update_reference())
        {
            --object->_M_references;
        }

        return *this;
    }
}// namespace Engine
