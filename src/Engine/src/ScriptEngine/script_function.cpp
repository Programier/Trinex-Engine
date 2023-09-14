#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{

    ScriptFunction::ScriptFunction(asIScriptFunction* function) : _M_context(nullptr), _M_function(function)
    {}


    ScriptFunction::ScriptFunction(const ScriptFunction& obj)
    {
        _M_function = obj._M_function;
        bind();
    }

    ScriptFunction::ScriptFunction(ScriptFunction&& obj)
    {
        _M_function     = obj._M_function;
        obj._M_function = nullptr;
    }

    ScriptFunction& ScriptFunction::operator=(ScriptFunction&& obj)
    {
        if (this != &obj)
        {
            unbind();
            _M_function     = obj._M_function;
            obj._M_function = nullptr;
        }
        return *this;
    }

    ScriptFunction& ScriptFunction::operator=(const ScriptFunction& obj)
    {
        if (this != &obj)
        {
            unbind();
            _M_function = obj._M_function;
            bind();
        }
        return *this;
    }

    bool ScriptFunction::operator==(const ScriptFunction& func) const
    {
        return _M_function == func._M_function;
    }

    bool ScriptFunction::operator!=(const ScriptFunction& func) const
    {
        return _M_function != func._M_function;
    }

    ScriptFunction& ScriptFunction::prepare()
    {
        if (_M_context == nullptr)
            _M_context = ScriptEngine::instance()->new_context();
        _M_context->Prepare(_M_function);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint8(uint_t arg, uint8_t value)
    {
        _M_context->SetArgByte(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint16(uint_t arg, uint16_t value)
    {
        _M_context->SetArgWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint32(uint_t arg, uint32_t value)
    {
        _M_context->SetArgDWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_uint64(uint_t arg, uint64_t value)
    {
        _M_context->SetArgQWord(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_float(uint_t arg, float value)
    {
        _M_context->SetArgFloat(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_double(uint_t arg, double value)
    {
        _M_context->SetArgDouble(arg, value);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_address(uint_t arg, void* addr)
    {
        _M_context->SetArgAddress(arg, addr);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_object(uint_t arg, void* obj)
    {
        _M_context->SetArgObject(arg, obj);
        return *this;
    }

    ScriptFunction& ScriptFunction::arg_var_type(uint_t arg, void* ptr, int_t type_id)
    {
        _M_context->SetArgVarType(arg, ptr, type_id);
        return *this;
    }

    ScriptFunction& ScriptFunction::object(const ScriptObject& object)
    {
        _M_context->SetObject(object._M_object);
        return *this;
    }

    ScriptFunction& ScriptFunction::call()
    {
        _M_context->Execute();
        return *this;
    }

    ScriptFunction& ScriptFunction::unbind_context()
    {
        if (_M_context)
        {
            ScriptEngine::instance()->release_context(_M_context);
            _M_context = nullptr;
        }
        return *this;
    }

    void* ScriptFunction::result_object_address()
    {
        return _M_context->GetReturnObject();
    }

    uint8_t ScriptFunction::result_byte()
    {
        return _M_context->GetReturnByte();
    }

    uint16_t ScriptFunction::result_word()
    {
        return _M_context->GetReturnWord();
    }

    uint32_t ScriptFunction::result_dword()
    {
        return _M_context->GetReturnDWord();
    }

    uint64_t ScriptFunction::result_qword()
    {
        return _M_context->GetReturnQWord();
    }

    float ScriptFunction::result_float()
    {
        return _M_context->GetReturnFloat();
    }

    double ScriptFunction::result_double()
    {
        return _M_context->GetReturnDouble();
    }

    void* ScriptFunction::result_address()
    {
        return _M_context->GetReturnAddress();
    }

    int_t ScriptFunction::id() const
    {
        return _M_function->GetId();
    }

    ScriptFunction::Type ScriptFunction::func_type() const
    {
        asEFuncType type = _M_function->GetFuncType();
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
        return _M_function->GetModuleName();
    }

    ScriptModule ScriptFunction::module() const
    {
        return ScriptModule(_M_function->GetModule());
    }

    const char* ScriptFunction::script_section_name() const
    {
        return _M_function->GetScriptSectionName();
    }

    ScriptTypeInfo ScriptFunction::object_type() const
    {
        return ScriptTypeInfo(_M_function->GetObjectType()).bind();
    }

    const char* ScriptFunction::object_name() const
    {
        return _M_function->GetObjectName();
    }

    const char* ScriptFunction::name() const
    {
        return _M_function->GetModuleName();
    }

    const char* ScriptFunction::namespace_name() const
    {
        return _M_function->GetNamespace();
    }

    const char* ScriptFunction::declaration(bool include_object_name, bool include_namespace,
                                            bool include_param_names) const
    {
        return _M_function->GetDeclaration(include_object_name, include_namespace, include_param_names);
    }

    bool ScriptFunction::is_read_only() const
    {
        return _M_function->IsReadOnly();
    }

    bool ScriptFunction::is_private() const
    {
        return _M_function->IsPrivate();
    }

    bool ScriptFunction::is_protected() const
    {
        return _M_function->IsProtected();
    }

    bool ScriptFunction::is_final() const
    {
        return _M_function->IsFinal();
    }

    bool ScriptFunction::is_override() const
    {
        return _M_function->IsOverride();
    }

    bool ScriptFunction::is_shared() const
    {
        return _M_function->IsShared();
    }

    bool ScriptFunction::is_explicit() const
    {
        return _M_function->IsExplicit();
    }

    bool ScriptFunction::is_property() const
    {
        return _M_function->IsProperty();
    }

    uint_t ScriptFunction::param_count() const
    {
        return _M_function->GetParamCount();
    }

    int_t ScriptFunction::type_id() const
    {
        return _M_function->GetTypeId();
    }

    bool ScriptFunction::is_compatible_with_type_id(int_t type_id) const
    {
        return _M_function->IsCompatibleWithTypeId(type_id);
    }

    // Delegates
    void* ScriptFunction::delegate_object() const
    {
        return _M_function->GetDelegateObject();
    }

    ScriptTypeInfo ScriptFunction::delegate_object_type() const
    {
        return ScriptTypeInfo(_M_function->GetDelegateObjectType()).bind();
    }

    ScriptFunction ScriptFunction::delegate_function() const
    {
        return ScriptFunction(_M_function->GetDelegateFunction()).bind();
    }

    ScriptFunction& ScriptFunction::bind()
    {
        if (_M_function)
        {
            _M_function->AddRef();
        }
        return *this;
    }

    ScriptFunction& ScriptFunction::unbind()
    {
        if (_M_function)
        {
            _M_function->Release();
            _M_function = nullptr;
        }
        return *this;
    }

    bool ScriptFunction::is_valid() const
    {
        return _M_function != nullptr;
    }

    ScriptFunction::~ScriptFunction()
    {
        unbind();
    }
}// namespace Engine
