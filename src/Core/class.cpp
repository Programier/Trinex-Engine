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

    Class::Class(const Name& name, Class* parent, BitMask _flags) : Struct(name, parent)
    {
        m_size = 0;
        info_log("Class", "Created class instance '%s'", this->name().c_str());
        flags               = _flags;
        m_cast_to_this      = nullptr;
        m_singletone_object = nullptr;

        if (is_asset())
        {
            get_asset_class_table().push_back(this);
        }

        if (parent)
        {
            parent->m_childs.insert(this);
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

    const Set<Class*>& Class::childs_classes() const
    {
        return m_childs;
    }

    void* Class::create_struct() const
    {
        return create_object();
    }

    Object* Class::create_object(StringView name, Object* owner) const
    {
        if (flags(Class::IsSingletone))
        {
            if (m_singletone_object == nullptr)
            {
                Object::setup_next_object_info(const_cast<Class*>(this));
                m_singletone_object = m_static_constructor(name, owner);
                Object::reset_next_object_info();
                on_create_call(m_singletone_object);
            }

            return m_singletone_object;
        }

        Object::setup_next_object_info(const_cast<Class*>(this));
        Object* object = m_static_constructor(name, owner);

        on_create_call(object);
        return object;
    }

    Object* Class::create_placement_object(void* place, StringView name, Object* owner) const
    {
        if (flags(Class::IsSingletone))
        {
            if (m_singletone_object == nullptr)
            {
                Object::setup_next_object_info(const_cast<Class*>(this));
                m_singletone_object = m_static_placement_constructor(place, name, owner);
                on_create_call(m_singletone_object);
                Object::reset_next_object_info();
                return m_singletone_object;
            }

            return nullptr;
        }

        Object::setup_next_object_info(const_cast<Class*>(this));
        Object* object = m_static_placement_constructor(place, name, owner);
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

    bool Class::is_scriptable() const
    {
        return flags(IsScriptable);
    }

    Object* (*Class::cast_to_this() const)(Object*)
    {
        return m_cast_to_this;
    }

    Class& Class::static_constructor(Object* (*new_static_constructor)(StringView, Object*) )
    {
        trinex_always_check(new_static_constructor, "Constructor can't be nullptr!");
        m_static_constructor = new_static_constructor;
        return *this;
    }

    Class& Class::static_placement_constructor(Object* (*new_static_placement_constructor)(void*, StringView, Object*) )
    {
        trinex_always_check(new_static_placement_constructor, "Constructor can't be nullptr!");
        m_static_placement_constructor = new_static_placement_constructor;
        return *this;
    }

    Object* Class::singletone_instance() const
    {
        return m_singletone_object;
    }

    Class& Class::post_initialize()
    {
        if (is_scriptable())
        {
            register_scriptable_class();
        }
        return *this;
    }

    bool Class::is_asset() const
    {
        return flags(Class::IsAsset);
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
    {
        on_class_destroy(this);

        if (Class* parent_class = parent())
        {
            parent_class->m_childs.erase(this);
        }
    }

    static void on_init()
    {
        ScriptClassRegistrar::reference_class("Engine::Class");
        ScriptBindingsInitializeController().push([]() {
            ScriptClassRegistrar::existing_class("Engine::Class")
                    .method("Class@ parent() const", &Class::parent)
                    .method("const string& name() const", &Class::name)
                    .method("const string& namespace_name() const", &Class::namespace_name)
                    .method("const string& base_name() const", &Class::base_name)
                    .static_function("Class@ static_find(const string& in)", Class::static_find)
                    .method("bool is_a(const Class@) const", method_of<bool, const Struct*>(&Struct::is_a))
                    .method("uint64 sizeof_class() const", &Class::sizeof_class)
                    .method("bool is_binded_to_script() const", &Class::is_scriptable)
                    .method("Object@ singletone_instance() const", &Class::singletone_instance);
        });
    }

    static ReflectionInitializeController initializer(on_init, "Engine::Class");
}// namespace Engine