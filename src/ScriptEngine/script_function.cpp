#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>

namespace Engine
{

#define check_function(return_value)                                                                                             \
    if (m_function == nullptr)                                                                                                   \
    return return_value

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
        check_function(0);
        return m_function->GetId();
    }

    ScriptFunction::Type ScriptFunction::type() const
    {
        check_function(Type::Dummy);

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

    StringView ScriptFunction::module_name() const
    {
        check_function("");
        return Strings::make_string_view(m_function->GetModuleName());
    }

    ScriptModule ScriptFunction::module() const
    {
        check_function({});
        return ScriptModule(m_function->GetModule());
    }

    StringView ScriptFunction::script_section_name() const
    {
        check_function("");
        return Strings::make_string_view(m_function->GetScriptSectionName());
    }

    ScriptTypeInfo ScriptFunction::object_type() const
    {
        check_function({});
        return ScriptTypeInfo(m_function->GetObjectType());
    }

    StringView ScriptFunction::object_name() const
    {
        check_function("");
        return Strings::make_string_view(m_function->GetObjectName());
    }

    StringView ScriptFunction::name() const
    {
        check_function("");
        return Strings::make_string_view(m_function->GetName());
    }

    StringView ScriptFunction::namespace_name() const
    {
        check_function("";) return Strings::make_string_view(m_function->GetNamespace());
    }

    String ScriptFunction::declaration(bool include_object_name, bool include_namespace, bool include_param_names) const
    {
        check_function("");
        return Strings::make_string(m_function->GetDeclaration(include_object_name, include_namespace, include_param_names));
    }

    bool ScriptFunction::is_read_only() const
    {
        check_function(false);
        return m_function->IsReadOnly();
    }

    bool ScriptFunction::is_private() const
    {
        check_function(false);
        return m_function->IsPrivate();
    }

    bool ScriptFunction::is_protected() const
    {
        return m_function->IsProtected();
    }

    bool ScriptFunction::is_final() const
    {
        check_function(false);
        return m_function->IsFinal();
    }

    bool ScriptFunction::is_override() const
    {
        check_function(false);
        return m_function->IsOverride();
    }

    bool ScriptFunction::is_shared() const
    {
        check_function(false);
        return m_function->IsShared();
    }

    bool ScriptFunction::is_explicit() const
    {
        check_function(false);
        return m_function->IsExplicit();
    }

    bool ScriptFunction::is_property() const
    {
        check_function(false);
        return m_function->IsProperty();
    }

    uint_t ScriptFunction::param_count() const
    {
        check_function(0);
        return m_function->GetParamCount();
    }

    bool ScriptFunction::param(uint_t index, int_t* type_id, Flags<ScriptTypeModifiers>* flags_ptr, StringView* name,
                               StringView* default_arg) const
    {
        check_function(false);
        const char* c_name        = nullptr;
        const char* c_default_arg = nullptr;
        asDWORD as_flags          = 0;
        bool status               = m_function->GetParam(index, type_id, &as_flags, &c_name, &c_default_arg) >= 0;

        if (status)
        {
            if (name && c_name)
            {
                (*name) = Strings::make_string_view(c_name);
            }

            if (default_arg && c_default_arg)
            {
                (*default_arg) = Strings::make_string_view(c_default_arg);
            }

            if (flags_ptr)
            {
                (*flags_ptr) = Flags<ScriptTypeModifiers>(as_flags);
            }
        }
        return status;
    }

    int_t ScriptFunction::return_type_id(Flags<ScriptTypeModifiers>* flags) const
    {
        check_function(0);
        asDWORD script_flags = 0;
        const int_t result   = m_function->GetReturnTypeId(flags ? &script_flags : nullptr);

        if (flags)
        {
            (*flags) = Flags<ScriptTypeModifiers>(script_flags);
        }

        return result;
    }

    int_t ScriptFunction::type_id() const
    {
        return m_function->GetTypeId();
    }

    bool ScriptFunction::is_compatible_with_type_id(int_t type_id) const
    {
        check_function(false);
        return m_function->IsCompatibleWithTypeId(type_id);
    }

    // Delegates
    void* ScriptFunction::delegate_object() const
    {
        check_function(nullptr);
        return m_function->GetDelegateObject();
    }

    ScriptTypeInfo ScriptFunction::delegate_object_type() const
    {
        check_function({});
        return ScriptTypeInfo(m_function->GetDelegateObjectType());
    }

    ScriptFunction ScriptFunction::delegate_function() const
    {
        check_function({});
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


    static void assign_script_function(asIScriptGeneric* generic)
    {
        ScriptFunction& function = *reinterpret_cast<ScriptFunction*>(generic->GetObject());
        auto info                = ScriptEngine::type_info_by_id(generic->GetArgTypeId(0));

        if (!info.is_valid())// Null ?
        {
            function = nullptr;
            return;
        }

        if (!info.is_funcdef())
        {
            error_log("ScriptFunction", "Cannot set new value. New value is not function handle!");
            return;
        }

        asIScriptFunction* script_function = *reinterpret_cast<asIScriptFunction**>(generic->GetArgAddress(0));

        if (script_function != nullptr)
        {
            function = script_function;
        }
    }

    static void construct_script_function(asIScriptGeneric* generic)
    {
        void* address = generic->GetObject();
        new (address) ScriptFunction();
        assign_script_function(generic);
    }

    static void on_init()
    {
        ScriptEnumRegistrar enum_registrar("Engine::ScriptFunction::Type");
        enum_registrar.set("Dummy", ScriptFunction::Type::Dummy);
        enum_registrar.set("System", ScriptFunction::Type::System);
        enum_registrar.set("Script", ScriptFunction::Type::Script);
        enum_registrar.set("Interface", ScriptFunction::Type::Interface);
        enum_registrar.set("Virtual", ScriptFunction::Type::Virtual);
        enum_registrar.set("Funcdef", ScriptFunction::Type::Funcdef);
        enum_registrar.set("Imported", ScriptFunction::Type::Imported);
        enum_registrar.set("Delegate", ScriptFunction::Type::Delegate);

        ScriptClassRegistrar::ClassInfo info = ScriptClassRegistrar::create_type_info<ScriptFunction>(
                ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassAllInts);

        ScriptClassRegistrar registrar("Engine::ScriptFunction", info);
        ReflectionInitializeController().require("Engine::ScriptTypeInfo").require("Engine::ScriptModule");

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<ScriptFunction>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const ScriptFunction& in)",
                         ScriptClassRegistrar::constructor<ScriptFunction, const ScriptFunction&>,
                         ScriptCallConv::CDECL_OBJFIRST);
        registrar.behave(ScriptClassBehave::Construct, "void f(const ?& in)", construct_script_function, ScriptCallConv::GENERIC);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<ScriptFunction>,
                         ScriptCallConv::CDECL_OBJFIRST);

        registrar.opfunc("bool opEquals(const ScriptFunction& in) const",
                         method_of<bool, const ScriptFunction&>(&ScriptFunction::operator==), ScriptCallConv::THISCALL);

        registrar.opfunc("ScriptFunction& opAssign(const ScriptFunction& in)",
                         method_of<ScriptFunction&, const ScriptFunction&>(&ScriptFunction::operator=), ScriptCallConv::THISCALL);
        registrar.opfunc("ScriptFunction& opAssign(const ?& in)", assign_script_function, ScriptCallConv::GENERIC);

        registrar.method("bool is_valid() const", &ScriptFunction::is_valid);
        registrar.method("int32 id() const", &ScriptFunction::id);
        registrar.method("ScriptFunction::Type type() const", &ScriptFunction::type);
        registrar.method("StringView module_name() const", &ScriptFunction::module_name);
        registrar.method("ScriptModule module() const", &ScriptFunction::module);
        registrar.method("StringView script_section_name() const", &ScriptFunction::script_section_name);
        registrar.method("StringView object_name() const", &ScriptFunction::object_name);
        registrar.method("ScriptTypeInfo object_type() const", &ScriptFunction::object_type);
        registrar.method("StringView name() const", &ScriptFunction::name);
        registrar.method("StringView namespace_name() const", &ScriptFunction::namespace_name);
        registrar.method("string declaration(bool include_object_name = true, bool include_namespace = false, bool "
                         "include_param_names = false) const",
                         &ScriptFunction::declaration);
        registrar.method("bool is_read_only() const", &ScriptFunction::is_read_only);
        registrar.method("bool is_private() const", &ScriptFunction::is_private);
        registrar.method("bool is_protected() const", &ScriptFunction::is_protected);
        registrar.method("bool is_final() const", &ScriptFunction::is_final);
        registrar.method("bool is_override() const", &ScriptFunction::is_override);
        registrar.method("bool is_shared() const", &ScriptFunction::is_shared);
        registrar.method("bool is_explicit() const", &ScriptFunction::is_explicit);
        registrar.method("bool is_property() const", &ScriptFunction::is_property);
        registrar.method("uint32 param_count() const", &ScriptFunction::param_count);
        registrar.method("int32 type_id() const", &ScriptFunction::type_id);
        registrar.method("bool is_compatible_with_type_id(int32 type_id) const", &ScriptFunction::is_compatible_with_type_id);
        registrar.method("ScriptTypeInfo delegate_object_type() const", &ScriptFunction::delegate_object_type);
        registrar.method("ScriptFunction delegate_function() const", &ScriptFunction::delegate_function);
        registrar.method("int32 return_type_id() const",
                         func_of<int_t(const ScriptFunction*)>(
                                 [](const ScriptFunction* self) -> int_t { return self->return_type_id(); }));
    }

    static ReflectionInitializeController initializer(on_init, "Engine::ScriptFunction", {"Engine::StringView"});
}// namespace Engine
