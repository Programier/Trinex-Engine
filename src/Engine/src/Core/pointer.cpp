#include <Core/archive.hpp>
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

    static FORCE_INLINE void serialize_reference(Archive& ar, Object*& object)
    {
        if (ar.is_saving())
        {
            String fullname = object ? object->full_name() : "";
            ar & fullname;
        }
        else
        {
            String fullname;
            ar & fullname;
            if (fullname.empty())
            {
                object = nullptr;
            }
            else
            {
                object = Object::load_object(fullname);
            }
        }
    }

    static FORCE_INLINE void serialize_object(Archive& ar, Object*& object)
    {
        trinex_always_check(object, "Cannot serialize nullptr object!");
        object->archive_process(ar);
    }

    bool PointerBase::archive_process(class Archive& ar, Object*& object, bool is_reference)
    {
        if (is_reference)
        {
            serialize_reference(ar, object);
        }
        else
        {
            serialize_object(ar, object);
        }

        return ar;
    }
}// namespace Engine
