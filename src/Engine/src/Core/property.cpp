#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>

namespace Engine
{
    Property::Property(const Name& name, const String& description, const Name& group, BitMask flags)
        : _M_name(name), _M_group(group), _M_description(description), _M_flags(flags)
    {}

    bool Property::is_valid_object(const class Class* instance, const class Object* object)
    {
        return object && object->class_instance()->is_a(instance);
    }

    const Name& Property::name() const
    {
        return _M_name;
    }

    const Name& Property::group() const
    {
        return _M_group;
    }

    const String& Property::description() const
    {
        return _M_description;
    }

    const Flags<Property::Flag>& Property::flags() const
    {
        return _M_flags;
    }

    void* Property::property_class() const
    {
        return nullptr;
    }

    Property::~Property()
    {}

    static bool serialize_object_properies(Class* self, Object* object, Archive& ar)
    {
        self = self->parent();
        if (self)
        {
            return serialize_object_properies(self, object, ar);
        }

        return ar;
    }

    bool Object::serialize_object_properties(Archive& ar)
    {
        Class* _class = class_instance();
        return serialize_object_properies(_class, this, ar);
    }
}// namespace Engine
