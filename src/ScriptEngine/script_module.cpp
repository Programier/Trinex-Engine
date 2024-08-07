#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_handle.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_primitives.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    ScriptModule::ScriptModule(const ScriptModule&) = default;

    ScriptModule::ScriptModule(ScriptModule&& other) : m_module(other.m_module)
    {
        other.m_module = nullptr;
    }

    ScriptModule& ScriptModule::operator=(ScriptModule&& other)
    {
        if (this != &other)
        {
            m_module       = other.m_module;
            other.m_module = nullptr;
        }
        return *this;
    }

    ScriptModule& ScriptModule::operator=(const ScriptModule&) = default;

    ScriptModule::ScriptModule(asIScriptModule* module) : m_module(module)
    {}

    ScriptModule::ScriptModule(const char* name, ModuleFlags flags)
    {
        (*this) = ScriptEngine::create_module(name, flags);
    }

    ScriptModule::ScriptModule(const String& name, ModuleFlags flags) : ScriptModule(name.c_str(), flags)
    {}

    asIScriptModule* ScriptModule::as_module() const
    {
        return m_module;
    }

    bool ScriptModule::is_valid() const
    {
        return m_module != nullptr;
    }

    ScriptModule& ScriptModule::name(const String& module_name)
    {
        return name(module_name.c_str());
    }

    ScriptModule& ScriptModule::name(const char* module_name)
    {
        m_module->SetName(module_name);
        return *this;
    }

    StringView ScriptModule::name() const
    {
        return Strings::make_string_view(m_module->GetName());
    }

    ScriptModule& ScriptModule::discard()
    {
        m_module->Discard();
        m_module = nullptr;
        return *this;
    }

    // Compilation
    bool ScriptModule::add_script_section(const char* section_name, const char* code, size_t code_length, int_t line_offset)
    {
        return m_module->AddScriptSection(section_name, code, code_length, line_offset) >= 0;
    }

    bool ScriptModule::add_script_section(const String& section_name, const String& code, size_t code_length, int_t line_offset)
    {
        return add_script_section(section_name.c_str(), code.c_str(), code_length, line_offset);
    }

    bool ScriptModule::build()
    {
        return m_module->Build() >= 0;
    }

    bool ScriptModule::compile_global_var(const char* section_name, const char* code, int_t line_offset)
    {
        return m_module->CompileGlobalVar(section_name, code, line_offset) >= 0;
    }

    bool ScriptModule::default_namespace(const char* name_space)
    {
        return m_module->SetDefaultNamespace(name_space) >= 0;
    }

    bool ScriptModule::default_namespace(const String& name_space)
    {
        return default_namespace(name_space.c_str());
    }

    const char* ScriptModule::default_namespace()
    {
        return m_module->GetDefaultNamespace();
    }

    uint_t ScriptModule::functions_count() const
    {
        return static_cast<Counter>(m_module->GetFunctionCount());
    }


    ScriptFunction ScriptModule::function_by_index(uint_t index) const
    {
        return ScriptFunction(m_module->GetFunctionByIndex(index));
    }

    ScriptFunction ScriptModule::function_by_decl(const char* decl) const
    {
        return ScriptFunction(m_module->GetFunctionByDecl(decl));
    }

    ScriptFunction ScriptModule::function_by_name(const char* func_name) const
    {
        return ScriptFunction(m_module->GetFunctionByName(func_name));
    }

    ScriptFunction ScriptModule::function_by_decl(const String& decl) const
    {
        return function_by_decl(decl.c_str());
    }

    ScriptFunction ScriptModule::function_by_name(const String& func_name) const
    {
        return function_by_name(func_name.c_str());
    }


    bool ScriptModule::remove_function(const ScriptFunction& function)
    {
        if (function.is_valid())
        {
            asIScriptFunction* func = function.function();
            function.release();
            return m_module->RemoveFunction(func) >= 0;
        }
        return false;
    }

    uint_t ScriptModule::global_var_count() const
    {
        return static_cast<Counter>(m_module->GetGlobalVarCount());
    }

    int_t ScriptModule::global_var_index_by_name(const char* module_name) const
    {
        return m_module->GetGlobalVarIndexByName(module_name);
    }

    int_t ScriptModule::global_var_index_by_decl(const char* decl) const
    {
        return m_module->GetGlobalVarIndexByDecl(decl);
    }

    int_t ScriptModule::global_var_index_by_name(const String& module_name) const
    {
        return m_module->GetGlobalVarIndexByName(module_name.c_str());
    }

    int_t ScriptModule::global_var_index_by_decl(const String& decl) const
    {
        return m_module->GetGlobalVarIndexByDecl(decl.c_str());
    }

    bool ScriptModule::global_var(uint_t index, StringView* name, StringView* name_space, int_t* type_id, bool* is_const) const
    {
        const char* out_name      = nullptr;
        const char* out_namespace = nullptr;
        bool result = m_module->GetGlobalVar(index, name ? &out_name : nullptr, name_space ? &out_namespace : nullptr, type_id,
                                             is_const) >= 0;

        if (result)
        {
            if (name)
            {
                (*name) = Strings::make_string_view(out_name);
            }

            if (name_space)
            {
                (*name_space) = Strings::make_string_view(out_namespace);
            }
        }

        return result;
    }

    String ScriptModule::global_var_declaration(uint_t index, bool include_namespace) const
    {
        return Strings::make_string(m_module->GetGlobalVarDeclaration(index, include_namespace));
    }

    void* ScriptModule::address_of_global_var(uint_t index)
    {
        return m_module->GetAddressOfGlobalVar(index);
    }

    int_t ScriptModule::remove_global_var(uint_t index)
    {
        return m_module->RemoveGlobalVar(index);
    }

    ScriptModule& ScriptModule::bind_imported_funcs()
    {
        m_module->BindAllImportedFunctions();
        return *this;
    }

    ScriptModule& ScriptModule::unbind_imported_funcs()
    {
        m_module->UnbindAllImportedFunctions();
        return *this;
    }

    uint_t ScriptModule::object_type_count() const
    {
        return m_module->GetObjectTypeCount();
    }

    ScriptTypeInfo ScriptModule::object_type_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_module->GetObjectTypeByIndex(index));
    }

    int_t ScriptModule::type_id_by_decl(const char* decl) const
    {
        return m_module->GetTypeIdByDecl(decl);
    }

    int_t ScriptModule::type_id_by_decl(const String& decl) const
    {
        return type_id_by_decl(decl.c_str());
    }

    ScriptTypeInfo ScriptModule::type_info_by_name(const char* name) const
    {
        return ScriptTypeInfo(m_module->GetTypeInfoByName(name));
    }

    ScriptTypeInfo ScriptModule::type_info_by_decl(const char* decl) const
    {
        return ScriptTypeInfo(m_module->GetTypeInfoByDecl(decl));
    }

    ScriptTypeInfo ScriptModule::type_info_by_name(const String& name) const
    {
        return type_info_by_name(name.c_str());
    }

    ScriptTypeInfo ScriptModule::type_info_by_decl(const String& decl) const
    {
        return type_info_by_decl(decl.c_str());
    }

    // Enums
    uint_t ScriptModule::enum_count() const
    {
        return m_module->GetEnumCount();
    }

    ScriptTypeInfo ScriptModule::enum_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_module->GetEnumByIndex(index));
    }

    // Typedefs
    uint_t ScriptModule::typedef_count() const
    {
        return m_module->GetTypedefCount();
    }

    ScriptTypeInfo ScriptModule::typedef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(m_module->GetTypedefByIndex(index));
    }

    class Script* ScriptModule::script() const
    {
        if (m_module)
        {
            return reinterpret_cast<Script*>(m_module->GetUserData(Constants::script_userdata_id));
        }
        return nullptr;
    }


    bool script_global_var(const ScriptModule* self, uint_t index, ScriptPointer* name, ScriptPointer* name_space,
                           Integer32* type_id, Boolean* is_const)
    {
        StringView* out_name      = name ? name->as<StringView>() : nullptr;
        StringView* out_namespace = name_space ? name_space->as<StringView>() : nullptr;
        int_t* out_type_id        = type_id ? &type_id->value : nullptr;
        bool* out_is_const        = is_const ? &is_const->value : nullptr;

        return self->global_var(index, out_name, out_namespace, out_type_id, out_is_const);
    }

    static void on_init()
    {
        {
            ScriptEnumRegistrar registrar("Engine::ScriptModule::ModuleFlags");
            registrar.set("OnlyIfExists", ScriptModule::ModuleFlags::OnlyIfExists);
            registrar.set("CreateIfNotExists", ScriptModule::ModuleFlags::CreateIfNotExists);
            registrar.set("AlwaysCreate", ScriptModule::ModuleFlags::AlwaysCreate);
        }

        ScriptClassRegistrar::ClassInfo info = ScriptClassRegistrar::create_type_info<ScriptModule>(
                ScriptClassRegistrar::Value | ScriptClassRegistrar::AppClassAllInts);

        ScriptClassRegistrar registrar("Engine::ScriptModule", info);
        ReflectionInitializeController()
                .require("Engine::ScriptFunction")
                .require("Engine::ScriptTypeInfo")
                .require("Engine::ScriptPointer");

        registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<ScriptModule>);
        registrar.behave(ScriptClassBehave::Construct, "void f(const ScriptModule& in)",
                         ScriptClassRegistrar::constructor<ScriptModule, const ScriptModule&>);
        registrar.behave(ScriptClassBehave::Construct,
                         "void f(const string& in name, Engine::ScriptModule::ModuleFlags flags = "
                         "Engine::ScriptModule::ModuleFlags::CreateIfNotExists)",
                         ScriptClassRegistrar::constructor<ScriptModule, const String&, ScriptModule::ModuleFlags>);
        registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<ScriptModule>);
        registrar.opfunc("ScriptModule& opAssign(const ScriptModule& in)",
                         method_of<ScriptModule&, const ScriptModule&>(&ScriptModule::operator=));

        registrar.method("bool is_valid() const", &ScriptModule::is_valid);
        registrar.method("ScriptModule& name(const string& in name)",
                         method_of<ScriptModule&, const String&>(&ScriptModule::name));
        registrar.method("StringView name() const", &ScriptModule::name);
        registrar.method("uint functions_count() const", &ScriptModule::functions_count);
        registrar.method("ScriptFunction function_by_index(uint index) const", &ScriptModule::function_by_index);
        registrar.method("ScriptFunction function_by_decl(const string& in decl) const",
                         method_of<ScriptFunction, const String&>(&ScriptModule::function_by_decl));
        registrar.method("ScriptFunction function_by_name(const string& in name) const",
                         method_of<ScriptFunction, const String&>(&ScriptModule::function_by_name));

        registrar.method("uint global_var_count() const", &ScriptModule::global_var_count);
        registrar.method("int global_var_index_by_name(const string& in) const",
                         method_of<int_t, const String&>(&ScriptModule::global_var_index_by_name));
        registrar.method("int global_var_index_by_decl(const string& in) const",
                         method_of<int_t, const String&>(&ScriptModule::global_var_index_by_decl));
        registrar.method("bool global_var(uint index, Ptr<StringView>@ name = null, Ptr<StringView>@ namespace_name = null, "
                         "Int32@ type_id = null, Bool@ is_const = null) const",
                         script_global_var);
        registrar.method("string global_var_declaration(uint index, bool include_namespace = false) const",
                         &ScriptModule::global_var_declaration);
        registrar.method("uint object_type_count() const", &ScriptModule::object_type_count);
        registrar.method("ScriptTypeInfo object_type_by_index(uint index) const", &ScriptModule::object_type_by_index);
        registrar.method("int type_id_by_decl(const string& in decl) const",
                         method_of<int_t, const String&>(&ScriptModule::type_id_by_decl));
        registrar.method("ScriptTypeInfo type_info_by_name(const string& in name) const",
                         method_of<ScriptTypeInfo, const String&>(&ScriptModule::type_info_by_name));
        registrar.method("ScriptTypeInfo type_info_by_decl(const string& in decl) const",
                         method_of<ScriptTypeInfo, const String&>(&ScriptModule::type_info_by_decl));

        registrar.method("uint enum_count() const", &ScriptModule::enum_count);
        registrar.method("ScriptTypeInfo enum_by_index(uint index) const", &ScriptModule::enum_by_index);
        registrar.method("uint typedef_count() const", &ScriptModule::typedef_count);
        registrar.method("ScriptTypeInfo typedef_by_index(uint index) const", &ScriptModule::typedef_by_index);
    }

    static ReflectionInitializeController initializer(on_init, "Engine::ScriptModule");

}// namespace Engine
