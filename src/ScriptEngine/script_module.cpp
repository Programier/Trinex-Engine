#include <Core/constants.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/string_functions.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
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

	ScriptModule::ScriptModule(asIScriptModule* module) : m_module(module) {}

	ScriptModule::ScriptModule(const char* name, ScriptModuleLookup lookup)
	{
		(*this) = ScriptEngine::module_by_name(name, lookup);
	}

	ScriptModule::ScriptModule(const String& name, ScriptModuleLookup lookup) : ScriptModule(name.c_str(), lookup) {}

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
		if (m_module)
		{
			m_module->Discard();
			m_module = nullptr;
		}
		return *this;
	}

	// Compilation
	bool ScriptModule::add_script_section(const char* section_name, const char* code, usize code_length, i32 line_offset)
	{
		return m_module->AddScriptSection(section_name, code, code_length, line_offset) >= 0;
	}

	bool ScriptModule::add_script_section(const String& section_name, const String& code, usize code_length, i32 line_offset)
	{
		return add_script_section(section_name.c_str(), code.c_str(), code_length, line_offset);
	}

	bool ScriptModule::build()
	{
		return m_module->Build() >= 0;
	}

	bool ScriptModule::compile_global_var(const char* section_name, const char* code, i32 line_offset)
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

	u32 ScriptModule::functions_count() const
	{
		return static_cast<usize>(m_module->GetFunctionCount());
	}


	ScriptFunction ScriptModule::function_by_index(u32 index) const
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

	u32 ScriptModule::global_var_count() const
	{
		return static_cast<usize>(m_module->GetGlobalVarCount());
	}

	i32 ScriptModule::global_var_index_by_name(const char* module_name) const
	{
		return m_module->GetGlobalVarIndexByName(module_name);
	}

	i32 ScriptModule::global_var_index_by_decl(const char* decl) const
	{
		return m_module->GetGlobalVarIndexByDecl(decl);
	}

	i32 ScriptModule::global_var_index_by_name(const String& module_name) const
	{
		return m_module->GetGlobalVarIndexByName(module_name.c_str());
	}

	i32 ScriptModule::global_var_index_by_decl(const String& decl) const
	{
		return m_module->GetGlobalVarIndexByDecl(decl.c_str());
	}

	bool ScriptModule::global_var(u32 index, StringView* name, StringView* name_space, i32* type_id, bool* is_const) const
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

	String ScriptModule::global_var_declaration(u32 index, bool include_namespace) const
	{
		return Strings::make_string(m_module->GetGlobalVarDeclaration(index, include_namespace));
	}

	void* ScriptModule::address_of_global_var(u32 index)
	{
		return m_module->GetAddressOfGlobalVar(index);
	}

	i32 ScriptModule::remove_global_var(u32 index)
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

	u32 ScriptModule::object_type_count() const
	{
		return m_module->GetObjectTypeCount();
	}

	ScriptTypeInfo ScriptModule::object_type_by_index(u32 index) const
	{
		return ScriptTypeInfo(m_module->GetObjectTypeByIndex(index));
	}

	i32 ScriptModule::type_id_by_decl(const char* decl) const
	{
		return m_module->GetTypeIdByDecl(decl);
	}

	i32 ScriptModule::type_id_by_decl(const String& decl) const
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
	u32 ScriptModule::enum_count() const
	{
		return m_module->GetEnumCount();
	}

	ScriptTypeInfo ScriptModule::enum_by_index(u32 index) const
	{
		return ScriptTypeInfo(m_module->GetEnumByIndex(index));
	}

	// Typedefs
	u32 ScriptModule::typedef_count() const
	{
		return m_module->GetTypedefCount();
	}

	ScriptTypeInfo ScriptModule::typedef_by_index(u32 index) const
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
}// namespace Engine
