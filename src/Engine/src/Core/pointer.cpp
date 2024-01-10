#include <Core/engine.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>

namespace Engine
{
    PointerBase::PointerBase() = default;

    PointerBase& PointerBase::add_reference(Object* object)
    {
        if (object && !engine_instance->is_shuting_down())
        {
            ++object->_M_references;
        }
        return *this;
    }

    PointerBase& PointerBase::remove_reference(Object* object)
    {
        if (object && !engine_instance->is_shuting_down())
        {
            --object->_M_references;
        }

        return *this;
    }
}// namespace Engine
