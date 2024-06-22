#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{

    ScriptFunction::ScriptFunction(asIScriptFunction* function) : m_context(nullptr), m_function(function)
    {}


    ScriptFunction::ScriptFunction(const ScriptFunction& obj)
    {
        m_function = obj.m_function;
        bind();
    }

    ScriptFunction::ScriptFunction(ScriptFunction&& obj)
    {
        m_function     = obj.m_function;
        obj.m_function = nullptr;
    }

    ScriptFunction& ScriptFunction::operator=(ScriptFunction&& obj)
    {
        if (this != &obj)
        {
            unbind();
            m_function     = obj.m_function;
            obj.m_function = nullptr;
        }
        return *this;
    }

    ScriptFunction& ScriptFunction::operator=(const ScriptFunction& obj)
    {
        if (this != &obj)
        {
            unbind();
            m_function = obj.m_function;
            bind();
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

    ScriptFunction& ScriptFunction::prepare()
    {
        if (m_context == nullptr)
            m_context = ScriptEngine::new_context();
        m_context->Prepare(m_function);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint8(uint_t arg, uint8_t value)
    {
        m_context->SetArgByte(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint16(uint_t arg, uint16_t value)
    {
        m_context->SetArgWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint32(uint_t arg, uint32_t value)
    {
        m_context->SetArgDWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint64(uint_t arg, uint64_t value)
    {
        m_context->SetArgQWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_float(uint_t arg, float value)
    {
        m_context->SetArgFloat(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_double(uint_t arg, double value)
    {
        m_context->SetArgDouble(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_address(uint_t arg, void* addr)
    {
        m_context->SetArgAddress(arg, addr);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_object(uint_t arg, void* obj)
    {
        m_context->SetArgObject(arg, obj);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_var_type(uint_t arg, void* ptr, int_t type_id)
    {
        m_context->SetArgVarType(arg, ptr, type_id);
        return *this;
    }

    ScriptFunction& ScriptFunction::object(const ScriptObject& object)
    {
        m_context->SetObject(object.m_object);
        return *this;
    }

    ScriptFunction& ScriptFunction::call()
    {
        m_context->Execute();
        return *this;
    }

    ScriptFunction& ScriptFunction::unbind_context()
    {
        if (m_context)
        {
            ScriptEngine::release_context(m_context);
            m_context = nullptr;
        }
        return *this;
    }

    void* ScriptFunction::result_object_address()
    {
        return m_context->GetReturnObject();
    }

    uint8_t ScriptFunction::result_byte()
    {
        return m_context->GetReturnByte();
    }

    uint16_t ScriptFunction::result_word()
    {
        return m_context->GetReturnWord();
    }

    uint32_t ScriptFunction::result_dword()
    {
        return m_context->GetReturnDWord();
    }

    uint64_t ScriptFunction::result_qword()
    {
        return m_context->GetReturnQWord();
    }

    float ScriptFunction::result_float()
    {
        return m_context->GetReturnFloat();
    }

    double ScriptFunction::result_double()
    {
        return m_context->GetReturnDouble();
    }

    void* ScriptFunction::result_address()
    {
        return m_context->GetReturnAddress();
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
        return ScriptTypeInfo(m_function->GetObjectType()).bind();
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
        return ScriptTypeInfo(m_function->GetDelegateObjectType()).bind();
    }

    ScriptFunction ScriptFunction::delegate_function() const
    {
        return ScriptFunction(m_function->GetDelegateFunction()).bind();
    }

    ScriptFunction& ScriptFunction::bind()
    {
        if (m_function)
        {
            m_function->AddRef();
        }
        return *this;
    }

    ScriptFunction& ScriptFunction::unbind()
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
        unbind();
    }
}// namespace Engine
