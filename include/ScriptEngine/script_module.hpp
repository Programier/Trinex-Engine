#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/string.hpp>
#include <ScriptEngine/enums.hpp>

class asIScriptModule;
class asITypeInfo;

namespace Trinex
{
	class ScriptFunction;
	class ScriptTypeInfo;
	class ScriptObject;

	class ENGINE_EXPORT ScriptModule
	{
	private:
		asIScriptModule* m_module;

	public:
		copy_constructors_hpp(ScriptModule);
		ScriptModule(asIScriptModule* module = nullptr);
		ScriptModule(const char* name, ScriptModuleLookup lookup = ScriptModuleLookup::CreateIfNotExists);
		ScriptModule(const String& name, ScriptModuleLookup lookup = ScriptModuleLookup::CreateIfNotExists);

		asIScriptModule* as_module() const;
		bool is_valid() const;
		ScriptModule& name(const String& name);
		ScriptModule& name(const char* name);
		StringView name() const;
		ScriptModule& discard();

		// Compilation
		bool add_script_section(const char* name, const char* code, usize code_length = 0, i32 line_offset = 0);
		bool add_script_section(const String& name, const String& code, usize code_length = 0, i32 line_offset = 0);
		bool build();

		bool compile_global_var(const char* section_name, const char* code, i32 line_offset);
		bool default_namespace(const char* name_space);
		bool default_namespace(const String& name_space);
		const char* default_namespace();

		// Functions
		u32 functions_count() const;
		ScriptFunction function_by_index(u32 index) const;
		ScriptFunction function_by_decl(const char* decl) const;
		ScriptFunction function_by_name(const char* name) const;
		ScriptFunction function_by_decl(const String& decl) const;
		ScriptFunction function_by_name(const String& name) const;
		bool remove_function(const ScriptFunction& function);

		// Global variables
		u32 global_var_count() const;
		i32 global_var_index_by_name(const char* name) const;
		i32 global_var_index_by_decl(const char* decl) const;
		i32 global_var_index_by_name(const String& name) const;
		i32 global_var_index_by_decl(const String& decl) const;
		bool global_var(u32 index, StringView* name = nullptr, StringView* name_space = nullptr, i32* type_id = nullptr,
		                bool* is_const = nullptr) const;
		String global_var_declaration(u32 index, bool include_namespace = false) const;
		void* address_of_global_var(u32 index);
		i32 remove_global_var(u32 index);

		ScriptModule& bind_imported_funcs();
		ScriptModule& unbind_imported_funcs();

		// Type identification
		u32 object_type_count() const;
		ScriptTypeInfo object_type_by_index(u32 index) const;
		i32 type_id_by_decl(const char* decl) const;
		i32 type_id_by_decl(const String& decl) const;
		ScriptTypeInfo type_info_by_name(const char* name) const;
		ScriptTypeInfo type_info_by_decl(const char* decl) const;
		ScriptTypeInfo type_info_by_name(const String& name) const;
		ScriptTypeInfo type_info_by_decl(const String& decl) const;

		// Enums
		u32 enum_count() const;
		ScriptTypeInfo enum_by_index(u32 index) const;

		// Typedefs
		u32 typedef_count() const;
		ScriptTypeInfo typedef_by_index(u32 index) const;

		class Script* script() const;

		friend class ScriptEngine;
		friend class ScriptTypeInfo;
		friend class ScriptFunction;
		friend class ScriptTypeInfo;
	};
}// namespace Trinex
