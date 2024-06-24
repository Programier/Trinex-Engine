#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    HashIndex ScriptObject::HashFunction::operator()(const ScriptObject& object) const
    {
        return reinterpret_cast<HashIndex>(object.m_object);
    }

    HashIndex ScriptObject::HashFunction::operator()(const ScriptObject* object) const
    {
        return reinterpret_cast<HashIndex>(object->m_object);
    }

    void ScriptObject::bind_script_functions()
    {
        if (m_object)
        {
            ScriptTypeInfo info = object_type();
            m_update            = info.method_by_decl("void update(float dt)");
            m_on_create         = info.method_by_decl("void on_create(Engine::Object@)");
        }
        else
        {
            m_update.unbind();
            m_on_create.unbind();
        }
    }

    ScriptObject::ScriptObject(asIScriptObject* object) : m_object(object)
    {
        bind_script_functions();
    }

    ScriptObject::ScriptObject(const char* name, bool uninited)
    {
        (*this) = ScriptModule::global().create_script_object(name, uninited);
    }

    ScriptObject::ScriptObject(const String& name, bool uninited) : ScriptObject(name.c_str(), uninited)
    {}

    ScriptObject& ScriptObject::remove_reference()
    {
        if (m_object)
        {
            ScriptEngine::destroy_script_object(m_object, object_type());
            m_object = nullptr;
            bind_script_functions();
        }
        return *this;
    }

    ScriptObject& ScriptObject::add_reference()
    {
        if (m_object)
        {
            m_object->AddRef();
        }
        return *this;
    }

    ScriptObject::ScriptObject(const ScriptObject& obj)
    {
        m_object = obj.m_object;
        add_reference();
        bind_script_functions();
    }

    ScriptObject::ScriptObject(ScriptObject&& obj)
    {
        m_object     = obj.m_object;
        obj.m_object = nullptr;
        bind_script_functions();
    }

    ScriptObject& ScriptObject::operator=(ScriptObject&& obj)
    {
        if (this != &obj)
        {
            remove_reference();
            m_object     = obj.m_object;
            obj.m_object = nullptr;
            bind_script_functions();
        }

        return *this;
    }

    ScriptObject& ScriptObject::operator=(const ScriptObject& obj)
    {
        if (this != &obj)
        {
            remove_reference();
            m_object = obj.m_object;
            add_reference();
            bind_script_functions();
        }

        return *this;
    }

    bool ScriptObject::is_valid() const
    {
        return m_object != nullptr;
    }

    int_t ScriptObject::type_id() const
    {
        return m_object->GetTypeId();
    }

    ScriptTypeInfo ScriptObject::object_type() const
    {
        return ScriptTypeInfo(m_object->GetObjectType());
    }

    // Class properties
    uint_t ScriptObject::property_count() const
    {
        return m_object->GetPropertyCount();
    }

    int_t ScriptObject::property_type_id(uint_t prop) const
    {
        return m_object->GetPropertyTypeId(prop);
    }

    const char* ScriptObject::property_name(uint_t prop) const
    {
        return m_object->GetPropertyName(prop);
    }

    void* ScriptObject::get_address_of_property(uint_t prop)
    {
        return m_object->GetAddressOfProperty(prop);
    }

    bool ScriptObject::operator==(const ScriptObject& other) const
    {
        return m_object == other.m_object;
    }

    bool ScriptObject::operator!=(const ScriptObject& other) const
    {
        return m_object != other.m_object;
    }

    bool ScriptObject::operator==(const asIScriptObject* other) const
    {
        return m_object == other;
    }

    bool ScriptObject::operator!=(const asIScriptObject* other) const
    {
        return m_object != other;
    }

    // Miscellaneous
    int_t ScriptObject::copy_from(const ScriptObject& other)
    {
        return m_object->CopyFrom(other.m_object);
    }

    ScriptObject::~ScriptObject()
    {
        remove_reference();
    }
}// namespace Engine
