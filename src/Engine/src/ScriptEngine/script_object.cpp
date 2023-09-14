#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    ScriptObject::ScriptObject(asIScriptObject* object) : _M_object(object)
    {}

    ScriptObject& ScriptObject::unbind()
    {
        if (_M_object)
        {
            ScriptEngine::instance()->destroy_script_object(_M_object, object_type());
            _M_object = nullptr;
        }

        return *this;
    }

    ScriptObject& ScriptObject::bind()
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
        bind();
    }

    ScriptObject::ScriptObject(ScriptObject&& obj)
    {
        _M_object     = obj._M_object;
        obj._M_object = nullptr;
    }

    ScriptObject& ScriptObject::operator=(ScriptObject&& obj)
    {
        if (this != &obj)
        {
            unbind();
            _M_object     = obj._M_object;
            obj._M_object = nullptr;
        }

        return *this;
    }

    ScriptObject& ScriptObject::operator=(const ScriptObject& obj)
    {
        if (this != &obj)
        {
            unbind();
            _M_object = obj._M_object;
            bind();
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

    // Miscellaneous
    int_t ScriptObject::copy_from(const ScriptObject& other)
    {
        return _M_object->CopyFrom(other._M_object);
    }

    ScriptObject::~ScriptObject()
    {
        unbind();
    }
}// namespace Engine
