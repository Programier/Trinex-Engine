#pragma once
#include <Core/enums.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>

class asIScriptContext;
class asIScriptFunction;

namespace Engine
{
	class ScriptTypeInfo;
	class ScriptModule;
	class ScriptEngine;
	class ScriptObject;

	class ENGINE_EXPORT ScriptFunction
	{
	public:
		enum Type
		{
			Dummy     = 0,
			System    = 1,
			Script    = 2,
			Interface = 3,
			Virtual   = 4,
			Funcdef   = 5,
			Imported  = 6,
			Delegate  = 7
		};

	private:
		mutable asIScriptFunction* m_function = nullptr;
		const ScriptFunction& add_ref() const;

	public:
		ScriptFunction(asIScriptFunction* function = nullptr);
		copy_constructors_hpp(ScriptFunction);


		asIScriptFunction* function() const;
		bool operator==(const ScriptFunction& func) const;
		bool operator!=(const ScriptFunction& func) const;

		bool is_valid() const;
		const ScriptFunction& release() const;

		int_t id() const;
		Type type() const;
		StringView module_name() const;
		ScriptModule module() const;
		StringView script_section_name() const;

		// Function signature
		ScriptTypeInfo object_type() const;
		StringView object_name() const;
		StringView name() const;
		StringView namespace_name() const;
		StringView config_group() const;
		String declaration(bool include_object_name = true, bool include_namespace = false,
		                   bool include_param_names = false) const;
		bool is_read_only() const;
		bool is_private() const;
		bool is_protected() const;
		bool is_final() const;
		bool is_override() const;
		bool is_shared() const;
		bool is_explicit() const;
		bool is_property() const;

		uint_t param_count() const;
		bool param(uint_t index, int_t* type_id, Flags<ScriptTypeModifiers>* flags = nullptr, StringView* name = nullptr,
		           StringView* default_arg = nullptr) const;
		int_t return_type_id(Flags<ScriptTypeModifiers>* flags = nullptr) const;

		// Type id for function pointers
		int_t type_id() const;
		bool is_compatible_with_type_id(int_t type_id) const;

		// Delegates
		void* delegate_object() const;
		ScriptTypeInfo delegate_object_type() const;
		ScriptFunction delegate_function() const;

		~ScriptFunction();
	};

	using ScriptClassMethod = ScriptFunction;
}// namespace Engine
