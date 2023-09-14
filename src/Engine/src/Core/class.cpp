#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>


namespace Engine
{
    static FORCE_INLINE Map<String, Class*>& class_table()
    {
        static Map<String, Class*> table;
        return table;
    }

    static PostDestroyController destroy([]() {
        for (auto& pair : class_table())
        {
            delete pair.second;
            pair.second = nullptr;
        }

        class_table().clear();
    });


    Object* Class::internal_cast(Class* required_class, Object* object)
    {
        if (object == nullptr)
        {
            return nullptr;
        }
        if (object->class_instance()->contains_class(required_class))
        {
            return object;
        }

        return nullptr;
    }

    Class::Class(const String& class_name, Object* (*static_constructor)(), Class* parent)
        : _M_name(class_name), _M_static_constructor(static_constructor), _M_parent(parent)
    {
        class_table()[class_name] = this;
        _M_size                   = 0;
        _M_flags                  = 0;
        info_log("Class", "Created class instance '%s'", class_name.c_str());

        _M_base_name            = Object::object_name_of(class_name);
        _M_namespace            = Object::package_name_of(class_name);
        _M_is_script_registered = false;
        _M_cast_to_this         = nullptr;
    }

    Class* Class::parent() const
    {
        return _M_parent;
    }

    const String& Class::name() const
    {
        return _M_name;
    }

    const String& Class::namespace_name() const
    {
        return _M_namespace;
    }

    const String& Class::base_name() const
    {
        return _M_base_name;
    }

    Object* Class::create_object() const
    {
        return _M_static_constructor();
    }

    bool Class::contains_class(const Class* c) const
    {
        const Class* current = this;
        while (current && current != c)
        {
            current = current->_M_parent;
        }
        return current != nullptr;
    }

    size_t Class::sizeof_class() const
    {
        return _M_size;
    }

    bool Class::has_any_flags(BitMask flags) const
    {
        return (_M_flags.to_ulong() & flags) != 0;
    }

    bool Class::has_all_flags(BitMask flags) const
    {
        return (_M_flags.to_ulong() & flags) == flags;
    }


    Class* Class::static_find_class(const String& name)
    {
        try
        {
            return class_table().at(name);
        }
        catch (...)
        {
            return nullptr;
        }
    }

    bool Class::is_binded_to_script() const
    {
        return _M_is_script_registered;
    }

    Object* (*Class::cast_to_this() const)(Object*)
    {
        return _M_cast_to_this;
    }

    Object* (*Class::static_constructor() const)()
    {
        return _M_static_constructor;
    }
}// namespace Engine
