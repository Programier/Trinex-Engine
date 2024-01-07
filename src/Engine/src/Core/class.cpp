#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>


namespace Engine
{
    static FORCE_INLINE Map<String, Class*>& get_class_table()
    {
        static Map<String, Class*> table;
        return table;
    }

    static FORCE_INLINE Vector<Class*>& get_asset_class_table()
    {
        static Vector<Class*> vector;
        return vector;
    }

    static PostDestroyController destroy([]() {
        for (auto& pair : get_class_table())
        {
            delete pair.second;
            pair.second = nullptr;
        }

        get_class_table().clear();
    });


    Object* Class::internal_cast(Class* required_class, Object* object)
    {
        if (object == nullptr)
        {
            return nullptr;
        }
        if (object->class_instance()->is_a(required_class))
        {
            return object;
        }

        return nullptr;
    }

    Class::Class(const String& class_name, Object* (*static_constructor)(), Class* parent, BitMask _flags)
        : _M_name(class_name), _M_static_constructor(static_constructor), _M_parent(parent), _M_singletone_object(nullptr)
    {
        get_class_table()[class_name] = this;
        _M_size                       = 0;
        info_log("Class", "Created class instance '%s'", class_name.c_str());

        _M_base_name          = Object::object_name_of(class_name);
        _M_namespace          = Object::package_name_of(class_name);
        flags.flags           = _flags;
        _M_cast_to_this       = nullptr;
        _M_base_name_splitted = Strings::make_sentence(_M_base_name);

        if (is_asset())
        {
            get_asset_class_table().push_back(this);
        }
    }

    Class* Class::parent() const
    {
        return _M_parent;
    }

    const String& Class::name() const
    {
        return _M_name;
    }

    const String& Class::base_name_splitted() const
    {
        return _M_base_name_splitted;
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
        if (flags(Class::IsSingletone))
        {
            if (_M_singletone_object == nullptr)
            {
                _M_singletone_object = _M_static_constructor();
            }

            return _M_singletone_object;
        }

        return _M_static_constructor();
    }

    bool Class::is_a(const Class* c) const
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
        return flags(IsScriptable);
    }

    Object* (*Class::cast_to_this() const)(Object*)
    {
        return _M_cast_to_this;
    }

    Object* (*Class::static_constructor() const)()
    {
        return _M_static_constructor;
    }

    Object* Class::singletone_instance() const
    {
        return _M_singletone_object;
    }

    Class& Class::add_property(class Property* prop)
    {
        if (!prop)
            return *this;

        _M_properties.push_back(prop);
        _M_grouped_property[prop->group()].push_back(prop);
        return *this;
    }

    const Map<String, Class*>& Class::class_table()
    {
        return get_class_table();
    }

    Class& Class::set_script_registration_callback(const CallBack<void(class ScriptClassRegistrar*, Class*)>& callback)
    {
        _M_script_register_callback = callback;
        return *this;
    }

    Class& Class::post_initialize()
    {
        if (is_binded_to_script())
        {
            ScriptClassRegistrar registrar(this);
            List<Class*> stack;

            Class* current = this;
            while (current)
            {
                stack.push_back(current);
                current = current->_M_parent;
            }

            while (!stack.empty())
            {
                current = stack.back();
                if (current->_M_script_register_callback)
                {
                    current->_M_script_register_callback(&registrar, this);
                }

                stack.pop_back();
            }
        }
        return *this;
    }

    bool Class::is_asset() const
    {
        const Class* self = this;
        while (self)
        {
            if (self->flags(Class::IsAsset))
            {
                return true;
            }

            self = self->_M_parent;
        }

        return false;
    }

    const Vector<Property*>& Class::properties() const
    {
        return _M_properties;
    }

    const Class::GroupedPropertiesMap& Class::grouped_properties() const
    {
        return _M_grouped_property;
    }

    const Vector<Class*>& Class::asset_classes()
    {
        return get_asset_class_table();
    }

    Class::~Class()
    {
        for (Property* prop : _M_properties)
        {
            delete prop;
        }

        _M_properties.clear();
    }

    static void on_init()
    {
        ScriptClassRegistrar::ClassInfo info;
        info.size  = sizeof(Class);
        info.flags = ScriptClassRegistrar::Ref | ScriptClassRegistrar::NoCount;
        ScriptClassRegistrar("Engine::Class", info)
                .require_types<Object>()
                .method("Class@ parent() const", &Class::parent)
                .method("const string& name() const", &Class::name)
                .method("const string& namespace_name() const", &Class::namespace_name)
                .method("const string& base_name() const", &Class::base_name)
                .method("Object@ create_object() const", &Class::create_object)
                .static_function("Class@ static_find_class(const string& in)", Class::static_find_class)
                .method("bool is_a(const Class@) const", method_of<bool, Class, const Class*>(&Class::is_a))
                .method("uint64 sizeof_class() const", &Class::sizeof_class)
                .method("bool is_binded_to_script() const", &Class::is_binded_to_script)
                .method("Object@ singletone_instance() const", &Class::singletone_instance);
    }

    static InitializeController initializer(on_init, "Bind Engine::Class");
}// namespace Engine
