#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    ScriptObject::ScriptObject(const ScriptObject& object) : ScriptVariableBase(object.address(), object.type_info(), true)
    {}

    ScriptObject& ScriptObject::operator=(const ScriptObject& object)
    {
        if (this == &object)
            return *this;

        if (!create(object.address(), object.type_info(), true))
        {
            error_log("ScriptObject", "Failed to create new object");
            release();
        }

        return *this;
    }

    ScriptObject::ScriptObject(const ScriptVariableBase& variable) : ScriptObject()
    {
        if (variable.is_object() || variable.is_handle())
        {
            if (!create(variable.address(), variable.type_info(), true))
            {
                error_log("ScriptObject", "Failed to convert script variable to script object!");
            }
        }
    }

    ScriptObject::ScriptObject(ScriptVariableBase&& variable)
    {
        if (variable.is_object() || variable.is_handle())
        {
            ScriptVariableBase::operator=(std::move(variable));
        }
    }

    ScriptObject& ScriptObject::operator=(const ScriptVariableBase& variable)
    {
        if (this == &variable)
            return *this;

        if (variable.is_object() || variable.is_handle())
        {
            if (!create(variable.address(), variable.type_info(), true))
            {
                error_log("ScriptObject", "Failed to convert script variable to script object!");
                release();
            }
        }
        return *this;
    }

    ScriptObject& ScriptObject::operator=(ScriptVariableBase&& variable)
    {
        if (variable.is_object() || variable.is_handle())
        {
            ScriptVariableBase::operator=(std::move(variable));
        }
        return *this;
    }

    uint_t ScriptObject::factory_count() const
    {
        return type_info().factory_count();
    }

    ScriptFunction ScriptObject::factory_by_index(uint_t index) const
    {
        return type_info().factory_by_index(index);
    }

    ScriptFunction ScriptObject::factory_by_decl(const char* decl) const
    {
        return type_info().factory_by_decl(decl);
    }

    ScriptFunction ScriptObject::factory_by_decl(const String& decl) const
    {
        return type_info().factory_by_decl(decl);
    }

    // Methods
    uint_t ScriptObject::method_count() const
    {
        return type_info().method_count();
    }

    ScriptFunction ScriptObject::method_by_index(uint_t index, bool get_virtual) const
    {
        return type_info().method_by_index(index, get_virtual);
    }

    ScriptFunction ScriptObject::method_by_name(const char* name, bool get_virtual) const
    {
        return type_info().method_by_name(name, get_virtual);
    }

    ScriptFunction ScriptObject::method_by_decl(const char* decl, bool get_virtual) const
    {
        return type_info().method_by_decl(decl, get_virtual);
    }

    ScriptFunction ScriptObject::method_by_name(const String& name, bool get_virtual) const
    {
        return type_info().method_by_name(name, get_virtual);
    }

    ScriptFunction ScriptObject::method_by_decl(const String& decl, bool get_virtual) const
    {
        return type_info().method_by_decl(decl, get_virtual);
    }

    // Properties
    uint_t ScriptObject::property_count() const
    {
        return type_info().property_count();
    }

    bool ScriptObject::property(uint_t index, StringView* name, int_t* type_id, bool* is_private, bool* is_protected, int_t* offset,
                                 bool* is_reference) const
    {
        return type_info().property(index, name, type_id, is_private, is_protected, offset, is_reference);
    }

    String ScriptObject::property_declaration(uint_t index, bool include_bamespace) const
    {
        return type_info().property_declaration(index, include_bamespace);
    }

    // Behaviours
    uint_t ScriptObject::behaviour_count() const
    {
        return type_info().behaviour_count();
    }

    ScriptFunction ScriptObject::behaviour_by_index(uint_t index, ScriptClassBehave* behaviour) const
    {
        return type_info().behaviour_by_index(index, behaviour);
    }

    ScriptTypeInfo ScriptObject::type_info() const
    {
        if (m_info.type_id() != type_id())
        {
            m_info = ScriptVariableBase::type_info();
        }
        return m_info;
    }
}// namespace Engine
