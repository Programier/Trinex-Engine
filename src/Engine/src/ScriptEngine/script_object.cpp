#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    HashIndex ScriptObject::HashFunction::operator()(const ScriptObject& object) const
    {
        return reinterpret_cast<HashIndex>(object._M_object);
    }

    HashIndex ScriptObject::HashFunction::operator()(const ScriptObject* object) const
    {
        return reinterpret_cast<HashIndex>(object->_M_object);
    }

    void ScriptObject::bind_script_functions()
    {
        if (_M_object)
        {
            ScriptTypeInfo info = object_type();
            _M_update           = info.method_by_decl("void update(float dt)");
            _M_on_create        = info.method_by_decl("void on_create(Engine::Object@)");
        }
        else
        {
            _M_update.unbind();
            _M_on_create.unbind();
        }
    }

    ScriptObject::ScriptObject(asIScriptObject* object) : _M_object(object)
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
        if (_M_object)
        {
            ScriptEngine::instance()->destroy_script_object(_M_object, object_type());
            _M_object = nullptr;
            bind_script_functions();
        }
        return *this;
    }

    ScriptObject& ScriptObject::add_reference()
    {
        if (_M_object)
        {
            _M_object->AddRef();
        }
        return *this;
    }

    ScriptObject::ScriptObject(const ScriptObject& obj)
    {
        _M_object = obj._M_object;
        add_reference();
        bind_script_functions();
    }

    ScriptObject::ScriptObject(ScriptObject&& obj)
    {
        _M_object     = obj._M_object;
        obj._M_object = nullptr;
        bind_script_functions();
    }

    ScriptObject& ScriptObject::operator=(ScriptObject&& obj)
    {
        if (this != &obj)
        {
            remove_reference();
            _M_object     = obj._M_object;
            obj._M_object = nullptr;
            bind_script_functions();
        }

        return *this;
    }

    ScriptObject& ScriptObject::operator=(const ScriptObject& obj)
    {
        if (this != &obj)
        {
            remove_reference();
            _M_object = obj._M_object;
            add_reference();
            bind_script_functions();
        }

        return *this;
    }

    bool ScriptObject::is_valid() const
    {
        return _M_object != nullptr;
    }

    int_t ScriptObject::type_id() const
    {
        return _M_object->GetTypeId();
    }

    ScriptTypeInfo ScriptObject::object_type() const
    {
        return ScriptTypeInfo(_M_object->GetObjectType()).bind();
    }

    // Class properties
    uint_t ScriptObject::property_count() const
    {
        return _M_object->GetPropertyCount();
    }

    int_t ScriptObject::property_type_id(uint_t prop) const
    {
        return _M_object->GetPropertyTypeId(prop);
    }

    const char* ScriptObject::property_name(uint_t prop) const
    {
        return _M_object->GetPropertyName(prop);
    }

    void* ScriptObject::get_address_of_property(uint_t prop)
    {
        return _M_object->GetAddressOfProperty(prop);
    }

    bool ScriptObject::operator==(const ScriptObject& other) const
    {
        return _M_object == other._M_object;
    }

    bool ScriptObject::operator!=(const ScriptObject& other) const
    {
        return _M_object != other._M_object;
    }

    bool ScriptObject::operator==(const asIScriptObject* other) const
    {
        return _M_object == other;
    }

    bool ScriptObject::operator!=(const asIScriptObject* other) const
    {
        return _M_object != other;
    }

    // Miscellaneous
    int_t ScriptObject::copy_from(const ScriptObject& other)
    {
        return _M_object->CopyFrom(other._M_object);
    }

    ScriptObject::~ScriptObject()
    {
        remove_reference();
    }

    void ScriptObject::update(float dt)
    {
        if (_M_update.is_valid())
        {
            _M_update.prepare().object(*this).arg_float(0, dt).call().unbind_context();
        }
    }

    void ScriptObject::on_create(Object* owner)
    {
        if (_M_on_create.is_valid())
        {
            _M_on_create.prepare().object(*this).arg_object(0, owner).call().unbind_context();
        }
    }
}// namespace Engine
