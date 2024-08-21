#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <ScriptEngine/script_variable.hpp>


class asIScriptObject;

namespace Engine
{
	class ScriptTypeInfo;
	class ScriptFunction;


	class ENGINE_EXPORT ScriptObject : public ScriptVariableBase
	{
		mutable ScriptTypeInfo m_info;

	public:
		using ScriptVariableBase::ScriptVariableBase;

		ScriptObject(void* address, int_t type_id, bool consider_handle_as_object = false);
		ScriptObject(void* address, const char* declaration, bool consider_handle_as_object = false);
		ScriptObject(void* address, const char* declaration, const char* module, bool consider_handle_as_object = false);
		ScriptObject(Object* self);

		ScriptObject(const ScriptObject& object);
		ScriptObject& operator=(const ScriptObject&);

		ScriptObject(const ScriptVariableBase& variable);
		ScriptObject(ScriptVariableBase&& variable);
		ScriptObject& operator=(const ScriptVariableBase& variable);
		ScriptObject& operator=(ScriptVariableBase&& variable);

		using ScriptVariableBase::create;
		bool create(int_t type_id, bool is_uninitialized = false);
		bool create(const char* type_declaration, bool is_uninitialized = false);
		bool create(const char* type_declaration, const char* module, bool is_uninitialized = false);
		bool create(void* src_address, int_t type_id, bool consider_handle_as_object = false);
		bool create(void* src_address, const char* type_declaration, bool consider_handle_as_object = false);
		bool create(void* src_address, const char* type_declaration, const char* module, bool consider_handle_as_object = false);
		bool create(Object* src);

		// Factories
		uint_t factory_count() const;
		ScriptFunction factory_by_index(uint_t index) const;
		ScriptFunction factory_by_decl(const char* decl) const;
		ScriptFunction factory_by_decl(const String& decl) const;

		// Methods
		uint_t method_count() const;
		ScriptFunction method_by_index(uint_t index, bool get_virtual = true) const;
		ScriptFunction method_by_name(const char* name, bool get_virtual = true) const;
		ScriptFunction method_by_decl(const char* decl, bool get_virtual = true) const;
		ScriptFunction method_by_name(const String& name, bool get_virtual = true) const;
		ScriptFunction method_by_decl(const String& decl, bool get_virtual = true) const;

		// Method execution

		template<typename... Args>
		ScriptVariable execute(const ScriptFunction& function, const Args&... args) const
		{
			return ScriptContext::execute<Args...>(*this, function, args...);
		}

		template<typename... Args>
		ScriptVariable execute(const char* method_name, const Args&... args) const
		{
			return execute(method_by_name(method_name), args...);
		}

		// Properties
		uint_t property_count() const;
		bool property(uint_t index, StringView* name = nullptr, int_t* type_id = nullptr, bool* is_private = nullptr,
		              bool* is_protected = nullptr, int_t* offset = nullptr, bool* is_reference = nullptr) const;
		String property_declaration(uint_t index, bool include_bamespace = false) const;

		// Behaviours
		uint_t behaviour_count() const;
		ScriptFunction behaviour_by_index(uint_t index, ScriptClassBehave* behaviour = nullptr) const;

		ScriptTypeInfo type_info() const override;
	};
}// namespace Engine
