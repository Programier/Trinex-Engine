#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/logger.hpp>
#include <Core/object.hpp>
#include <Core/property.hpp>


namespace Engine
{
    static FORCE_INLINE Vector<Class*>& get_asset_class_table()
    {
        static Vector<Class*> vector;
        return vector;
    }

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

    Class::Class(const Name& name, const Name& namespace_name, Object* (*constructor)(), Class* parent, BitMask _flags)
        : Struct(name, namespace_name, parent)
    {
        m_size = 0;
        info_log("Class", "Created class instance '%s'", this->name().c_str());
        flags                = _flags;
        m_cast_to_this       = nullptr;
        m_static_constructor = constructor;
        m_singletone_object  = nullptr;

        if (is_asset())
        {
            get_asset_class_table().push_back(this);
        }
    }

    void Class::on_create_call(Object* object) const
    {
        if (Class* parent_class = parent())
        {
            parent_class->on_create_call(object);
        }

        on_create(object);
    }

    Class* Class::parent() const
    {
        return reinterpret_cast<Class*>(Struct::parent());
    }

    void* Class::create_struct() const
    {
        return create_object();
    }

    Object* Class::create_object() const
    {
        if (flags(Class::IsSingletone))
        {
            if (m_singletone_object == nullptr)
            {
                m_singletone_object = m_static_constructor();
                on_create_call(m_singletone_object);
            }

            return m_singletone_object;
        }

        Object* object = m_static_constructor();
        on_create_call(object);
        return object;
    }

    size_t Class::sizeof_class() const
    {
        return m_size;
    }

    Class* Class::static_find(const StringView& name, bool required)
    {
        Struct* self = Struct::static_find(name, required);
        if (self && self->is_class())
        {
            return reinterpret_cast<Class*>(self);
        }

        return nullptr;
    }

    bool Class::is_binded_to_script() const
    {
        return flags(IsScriptable);
    }

    Object* (*Class::cast_to_this() const)(Object*)
    {
        return m_cast_to_this;
    }

    Object* (*Class::static_constructor() const)()
    {
        return m_static_constructor;
    }

    Object* Class::singletone_instance() const
    {
        return m_singletone_object;
    }

    Class& Class::set_script_registration_callback(const CallBack<void(class ScriptClassRegistrar*, Class*)>& callback)
    {
        m_script_register_callback = callback;
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
                current = current->parent();
            }

            while (!stack.empty())
            {
                current = stack.back();
                if (current->m_script_register_callback)
                {
                    current->m_script_register_callback(&registrar, this);
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

            self = self->parent();
        }

        return false;
    }

    bool Class::is_class() const
    {
        return true;
    }

    const Vector<Class*>& Class::asset_classes()
    {
        return get_asset_class_table();
    }

    Class::~Class()
    {}

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
                .static_function("Class@ static_find(const string& in)", Class::static_find)
                .method("bool is_a(const Class@) const", method_of<bool, Class, const Struct*>(&Class::is_a))
                .method("uint64 sizeof_class() const", &Class::sizeof_class)
                .method("bool is_binded_to_script() const", &Class::is_binded_to_script)
                .method("Object@ singletone_instance() const", &Class::singletone_instance);
    }

    static ScriptEngineInitializeController initializer(on_init, "Bind Engine::Class");
}// namespace Engine
