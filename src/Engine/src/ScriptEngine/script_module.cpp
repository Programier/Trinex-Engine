#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
    default_copy_constructors_cpp(ScriptModule);

    ScriptModule::ScriptModule(asIScriptModule* module) : _M_module(module)
    {}

    ScriptModule ScriptModule::global()
    {
        return ScriptEngine::instance()->global_module();
    }

    bool ScriptModule::is_valid() const
    {
        return _M_module != nullptr;
    }

    ScriptModule& ScriptModule::name(const String& module_name)
    {
        return name(module_name.c_str());
    }

    ScriptModule& ScriptModule::name(const char* module_name)
    {
        _M_module->SetName(module_name);
        return *this;
    }

    const char* ScriptModule::name() const
    {
        return _M_module->GetName();
    }

    ScriptModule& ScriptModule::discard()
    {
        _M_module->Discard();
        return *this;
    }

    // Compilation
    int_t ScriptModule::add_script_section(const char* section_name, const char* code, size_t code_length, int_t line_offset)
    {
        return static_cast<int_t>(_M_module->AddScriptSection(section_name, code, code_length, line_offset));
    }

    int_t ScriptModule::add_script_section(const String& section_name, const String& code, size_t code_length, int line_offset)
    {
        return add_script_section(section_name.c_str(), code.c_str(), code_length, line_offset);
    }

    int_t ScriptModule::build()
    {
        return static_cast<int_t>(_M_module->Build());
    }

    int_t ScriptModule::compile_global_var(const char* section_name, const char* code, int line_offset)
    {
        return static_cast<int_t>(_M_module->CompileGlobalVar(section_name, code, line_offset));
    }

    ScriptModule& ScriptModule::default_namespace(const char* name_space)
    {
        _M_module->SetDefaultNamespace(name_space);
        return *this;
    }

    ScriptModule& ScriptModule::default_namespace(const String& name_space)
    {
        return default_namespace(name_space.c_str());
    }

    const char* ScriptModule::default_namespace()
    {
        return _M_module->GetDefaultNamespace();
    }

    Counter ScriptModule::functions_count() const
    {
        return static_cast<Counter>(_M_module->GetFunctionCount());
    }


    ScriptFunction ScriptModule::function_by_index(Index index) const
    {
        return ScriptFunction(_M_module->GetFunctionByIndex(index)).bind();
    }

    ScriptFunction ScriptModule::function_by_decl(const char* decl) const
    {
        return ScriptFunction(_M_module->GetFunctionByDecl(decl)).bind();
    }

    ScriptFunction ScriptModule::function_by_name(const char* func_name) const
    {
        return ScriptFunction(_M_module->GetFunctionByName(func_name)).bind();
    }

    ScriptFunction ScriptModule::function_by_decl(const String& decl) const
    {
        return function_by_decl(decl.c_str());
    }

    ScriptFunction ScriptModule::function_by_name(const String& func_name) const
    {
        return function_by_name(func_name.c_str());
    }


    int_t ScriptModule::remove_function(const ScriptFunction& function)
    {
        if (function.is_valid())
        {
            int_t result = static_cast<int_t>(_M_module->RemoveFunction(function._M_function));
            if (result >= 0)
            {
                const_cast<ScriptFunction&>(function)._M_function      = nullptr;
                const_cast<ScriptFunction&>(function).unbind_context() = nullptr;
            }

            return result;
        }
        return -1;
    }

    Counter ScriptModule::global_var_count() const
    {
        return static_cast<Counter>(_M_module->GetGlobalVarCount());
    }

    int_t ScriptModule::global_var_index_by_name(const char* module_name) const
    {
        return _M_module->GetGlobalVarIndexByName(module_name);
    }

    int_t ScriptModule::global_var_index_by_decl(const char* decl) const
    {
        return _M_module->GetGlobalVarIndexByDecl(decl);
    }

    int_t ScriptModule::global_var_index_by_name(const String& module_name) const
    {
        return _M_module->GetGlobalVarIndexByName(module_name.c_str());
    }

    int_t ScriptModule::global_var_index_by_decl(const String& decl) const
    {
        return _M_module->GetGlobalVarIndexByDecl(decl.c_str());
    }

    const char* ScriptModule::global_var_declaration(uint_t index, bool include_namespace) const
    {
        return _M_module->GetGlobalVarDeclaration(index, include_namespace);
    }

    void* ScriptModule::address_of_global_var(uint_t index)
    {
        return _M_module->GetAddressOfGlobalVar(index);
    }

    int_t ScriptModule::remove_global_var(uint_t index)
    {
        return _M_module->RemoveGlobalVar(index);
    }

    ScriptModule& ScriptModule::bind_imported_funcs()
    {
        _M_module->BindAllImportedFunctions();
        return *this;
    }

    ScriptModule& ScriptModule::unbind_imported_funcs()
    {
        _M_module->UnbindAllImportedFunctions();
        return *this;
    }

    uint_t ScriptModule::object_type_count() const
    {
        return _M_module->GetObjectTypeCount();
    }

    ScriptTypeInfo ScriptModule::object_type_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_module->GetObjectTypeByIndex(index)).bind();
    }

    int_t ScriptModule::type_id_by_decl(const char* decl) const
    {
        return _M_module->GetTypeIdByDecl(decl);
    }

    int_t ScriptModule::type_id_by_decl(const String& decl) const
    {
        return type_id_by_decl(decl.c_str());
    }

    ScriptTypeInfo ScriptModule::type_info_by_name(const char* name) const
    {
        return ScriptTypeInfo(_M_module->GetTypeInfoByName(name)).bind();
    }

    ScriptTypeInfo ScriptModule::type_info_by_decl(const char* decl) const
    {
        return ScriptTypeInfo(_M_module->GetTypeInfoByDecl(decl)).bind();
    }

    ScriptTypeInfo ScriptModule::type_info_by_name(const String& name) const
    {
        return type_info_by_name(name.c_str());
    }

    ScriptTypeInfo ScriptModule::type_info_by_decl(const String& decl) const
    {
        return type_info_by_decl(decl.c_str());
    }


    ScriptObject ScriptModule::create_script_object(const ScriptTypeInfo& type_info, bool uninited)
    {
        return ScriptEngine::instance()->create_script_object(type_info, uninited);
    }

    ScriptObject ScriptModule::create_script_object(const char* class_name, bool uninited)
    {
        return create_script_object(type_info_by_name(class_name), uninited);
    }

    ScriptObject ScriptModule::create_script_object(const String& name, bool uninited)
    {
        return create_script_object(name.c_str(), uninited);
    }

    //        // Enums
    uint_t ScriptModule::enum_count() const
    {
        return _M_module->GetEnumCount();
    }

    ScriptTypeInfo ScriptModule::enum_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_module->GetEnumByIndex(index)).bind();
    }

    //        // Typedefs
    uint_t ScriptModule::typedef_count() const
    {
        return _M_module->GetTypedefCount();
    }

    ScriptTypeInfo ScriptModule::typedef_by_index(uint_t index) const
    {
        return ScriptTypeInfo(_M_module->GetTypedefByIndex(index)).bind();
    }
}// namespace Engine
