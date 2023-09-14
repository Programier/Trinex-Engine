#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>


class asIScriptModule;
class asITypeInfo;

namespace Engine
{
    class ScriptFunction;
    class ScriptTypeInfo;

    class ENGINE_EXPORT ScriptModule
    {
    private:
        asIScriptModule* _M_module;

    public:
        copy_constructors_hpp(ScriptModule);
        ScriptModule(asIScriptModule* module = nullptr);
        bool is_valid() const;

        ScriptModule& name(const String& name);
        ScriptModule& name(const char* name);
        const char* name() const;

        ScriptModule& discard();

        // Compilation
        int_t add_script_section(const char* name, const char* code, size_t code_length = 0, int_t line_offset = 0);
        int_t add_script_section(const String& name, const String& code, size_t code_length = 0, int line_offset = 0);
        int_t build();

        int_t compile_global_var(const char* section_name, const char* code, int line_offset);
        ScriptModule& default_namespace(const char* name_space);
        ScriptModule& default_namespace(const String& name_space);
        const char* default_namespace();

        //        // Functions
        Counter functions_count() const;
        ScriptFunction function_by_index(Index index) const;
        ScriptFunction function_by_decl(const char* decl) const;
        ScriptFunction function_by_name(const char* name) const;
        ScriptFunction function_by_decl(const String& decl) const;
        ScriptFunction function_by_name(const String& name) const;
        int_t remove_function(const ScriptFunction& function);

        //        // Global variables
        Counter global_var_count() const;
        int_t global_var_index_by_name(const char* name) const;
        int_t global_var_index_by_decl(const char* decl) const;
        int_t global_var_index_by_name(const String& name) const;
        int_t global_var_index_by_decl(const String& decl) const;
        const char* global_var_declaration(uint_t index, bool include_namespace = false) const;
        void* address_of_global_var(uint_t index);
        int_t remove_global_var(uint_t index);

        ScriptModule& bind_imported_funcs();
        ScriptModule& unbind_imported_funcs();

        // Type identification
        uint_t object_type_count() const;
        ScriptTypeInfo object_type_by_index(uint_t index) const;
        int_t type_id_by_decl(const char* decl) const;
        int_t type_id_by_decl(const String& decl) const;
        ScriptTypeInfo type_info_by_name(const char* name) const;
        ScriptTypeInfo type_info_by_decl(const char* decl) const;
        ScriptTypeInfo type_info_by_name(const String& name) const;
        ScriptTypeInfo type_info_by_decl(const String& decl) const;

        //        // Enums
        uint_t enum_count() const;
        ScriptTypeInfo enum_by_index(uint_t index) const;

        //        // Typedefs
        uint_t typedef_count() const;
        ScriptTypeInfo typedef_by_index(uint_t index) const;

        friend class ScriptEngine;
        friend class ScriptTypeInfo;
        friend class ScriptFunction;
        friend class ScriptTypeInfo;
    };
}// namespace Engine
