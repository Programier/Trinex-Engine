#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{

    ScriptFunction::ScriptFunction(asIScriptFunction* function) : m_function(function)
    {
        add_ref();
    }

    ScriptFunction::ScriptFunction(const ScriptFunction& obj) : Engine::ScriptFunction(obj.function())
    {}

    ScriptFunction::ScriptFunction(ScriptFunction&& obj)
    {
        m_function     = obj.m_function;
        obj.m_function = nullptr;
    }

    asIScriptFunction* ScriptFunction::function() const
    {
        return m_function;
    }

    ScriptFunction& ScriptFunction::operator=(ScriptFunction&& obj)
    {
        if (this != &obj)
        {
            release();
            m_function     = obj.m_function;
            obj.m_function = nullptr;
        }
        return *this;
    }

    ScriptFunction& ScriptFunction::operator=(const ScriptFunction& obj)
    {
        if (this != &obj)
        {
            release();
            m_function = obj.m_function;
            add_ref();
        }
        return *this;
    }

    bool ScriptFunction::operator==(const ScriptFunction& func) const
    {
        return m_function == func.m_function;
    }

    bool ScriptFunction::operator!=(const ScriptFunction& func) const
    {
        return m_function != func.m_function;
    }

    int_t ScriptFunction::id() const
    {
        return m_function->GetId();
    }

    ScriptFunction::Type ScriptFunction::func_type() const
    {
        asEFuncType type = m_function->GetFuncType();
        switch (type)
        {
            case asFUNC_DUMMY:
                return Type::Dummy;
            case asFUNC_SYSTEM:
                return Type::System;
            case asFUNC_SCRIPT:
                return Type::Script;
            case asFUNC_INTERFACE:
                return Type::Interface;
            case asFUNC_VIRTUAL:
                return Type::Virtual;
            case asFUNC_FUNCDEF:
                return Type::Funcdef;
            case asFUNC_IMPORTED:
                return Type::Imported;
            case asFUNC_DELEGATE:
                return Type::Delegate;
            default:
                return Type::Dummy;
        }
    }

    const char* ScriptFunction::module_name() const
    {
        return m_function->GetModuleName();
    }

    ScriptModule ScriptFunction::module() const
    {
        return ScriptModule(m_function->GetModule());
    }

    const char* ScriptFunction::script_section_name() const
    {
        return m_function->GetScriptSectionName();
    }

    ScriptTypeInfo ScriptFunction::object_type() const
    {
        return ScriptTypeInfo(m_function->GetObjectType());
    }

    const char* ScriptFunction::object_name() const
    {
        return m_function->GetObjectName();
    }

    const char* ScriptFunction::name() const
    {
        return m_function->GetName();
    }

    const char* ScriptFunction::namespace_name() const
    {
        return m_function->GetNamespace();
    }

    const char* ScriptFunction::declaration(bool include_object_name, bool include_namespace, bool include_param_names) const
    {
        return m_function->GetDeclaration(include_object_name, include_namespace, include_param_names);
    }

    bool ScriptFunction::is_read_only() const
    {
        return m_function->IsReadOnly();
    }

    bool ScriptFunction::is_private() const
    {
        return m_function->IsPrivate();
    }

    bool ScriptFunction::is_protected() const
    {
        return m_function->IsProtected();
    }

    bool ScriptFunction::is_final() const
    {
        return m_function->IsFinal();
    }

    bool ScriptFunction::is_override() const
    {
        return m_function->IsOverride();
    }

    bool ScriptFunction::is_shared() const
    {
        return m_function->IsShared();
    }

    bool ScriptFunction::is_explicit() const
    {
        return m_function->IsExplicit();
    }

    bool ScriptFunction::is_property() const
    {
        return m_function->IsProperty();
    }

    uint_t ScriptFunction::param_count() const
    {
        return m_function->GetParamCount();
    }

    int_t ScriptFunction::type_id() const
    {
        return m_function->GetTypeId();
    }

    bool ScriptFunction::is_compatible_with_type_id(int_t type_id) const
    {
        return m_function->IsCompatibleWithTypeId(type_id);
    }

    // Delegates
    void* ScriptFunction::delegate_object() const
    {
        return m_function->GetDelegateObject();
    }

    ScriptTypeInfo ScriptFunction::delegate_object_type() const
    {
        return ScriptTypeInfo(m_function->GetDelegateObjectType());
    }

    ScriptFunction ScriptFunction::delegate_function() const
    {
        return ScriptFunction(m_function->GetDelegateFunction());
    }

    const ScriptFunction& ScriptFunction::add_ref() const
    {
        if (m_function)
        {
            m_function->AddRef();
        }
        return *this;
    }

    const ScriptFunction& ScriptFunction::release() const
    {
        if (m_function)
        {
            m_function->Release();
            m_function = nullptr;
        }
        return *this;
    }

    bool ScriptFunction::is_valid() const
    {
        return m_function != nullptr;
    }

    ScriptFunction::~ScriptFunction()
    {
        release();
    }
}// namespace Engine
