#include <Core/class.hpp>
#include <Core/class_members.hpp>
#include <Core/engine_loading_controllers.hpp>


namespace Engine
{
    ClassField::ClassField(ClassFieldOffset offset, AccessType access_type, bool is_seriazable)
        : _M_offset(offset), _M_access_type(access_type), _M_is_serializable(is_seriazable)
    {}

    ClassField::ClassField()
    {}


    ClassFieldOffset ClassField::offset() const
    {
        return _M_offset;
    }

    byte* ClassField::from(Object* object) const
    {
        if (object->package() != package())
        {
            return nullptr;
        }

        return reinterpret_cast<byte*>(object) + _M_offset;
    }

    const byte* ClassField::from(const Object* object) const
    {
        if (object->package() != package())
        {
            return nullptr;
        }

        return reinterpret_cast<const byte*>(object) + _M_offset;
    }

    AccessType ClassField::access_type() const
    {
        return _M_access_type;
    }

    bool ClassField::is_serializable() const
    {
        return _M_is_serializable;
    }

    bool ClassField::archive_process_from(Archive* ar, Object* object)
    {
        return ((*this).*_M_archive_process)(ar, object);
    }

    implement_class(ClassField, "Engine");
    implement_default_initialize_class(ClassField);
}// namespace Engine
