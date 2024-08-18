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

#define check_function(return_value)                                                                                   \
	if (m_function == nullptr)                                                                                         \
	return return_value

	ScriptFunction::ScriptFunction(asIScriptFunction* function) : m_function(function)
	{
		add_ref();
	}

	ScriptFunction::ScriptFunction(const ScriptFunction& obj) : Engine::ScriptFunction(obj.function())
	{}

	ScriptFunction::ScriptFunction(ScriptFunction&& obj)
	{
		m_function	   = obj.m_function;
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
			m_function	   = obj.m_function;
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
		return Strings::make_string(
				m_function->GetDeclaration(include_object_name, include_namespace, include_param_names));
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
		const char* c_name		  = nullptr;
		const char* c_default_arg = nullptr;
		asDWORD as_flags		  = 0;
		bool status				  = m_function->GetParam(index, type_id, &as_flags, &c_name, &c_default_arg) >= 0;

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
		const int_t result	 = m_function->GetReturnTypeId(flags ? &script_flags : nullptr);

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
}// namespace Engine
