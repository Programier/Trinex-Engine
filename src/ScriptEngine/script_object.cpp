#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <angelscript.h>


namespace Engine
{
	static FORCE_INLINE void* object_from_handle(void* address, bool is_object_address_for_handle)
	{
		if (address == nullptr)
			return address;

		if (is_object_address_for_handle)
			return address;

		return *reinterpret_cast<void**>(address);
	}

	static FORCE_INLINE int_t find_type_id(const char* declaration, const char* module_name)
	{
		auto module = ScriptModule(module_name, ScriptModule::OnlyIfExists);

		if (module.is_valid())
			return module.type_id_by_decl(declaration);

		return 0;
	}

	static FORCE_INLINE int_t find_type_id(const char* declaration)
	{
		return ScriptEngine::type_id_by_decl(declaration);
	}

	ScriptObject::ScriptObject(void* address, int_t type_id, bool consider_handle_as_object)
	{
		if (ScriptEngine::is_object_type(type_id, true))
		{
			create(address, type_id, consider_handle_as_object);
		}
		else
		{
			error_log("ScriptObject", "Type ID '%d' is not class type id!", type_id);
		}
	}

	ScriptObject::ScriptObject(void* address, const char* declaration, bool consider_handle_as_object)
		: Engine::ScriptObject(address, find_type_id(declaration), consider_handle_as_object)
	{}

	ScriptObject::ScriptObject(void* address, const char* declaration, const char* module, bool consider_handle_as_object)
		: Engine::ScriptObject(address, find_type_id(declaration, module), consider_handle_as_object)
	{}

	ScriptObject::ScriptObject(const ScriptObject& object) : ScriptVariableBase(object.address(), object.type_info(), true)
	{}

	ScriptObject& ScriptObject::operator=(const ScriptObject& object)
	{
		if (this == &object)
			return *this;

		if (!create(object.address(), object.type_id(), true))
		{
			error_log("ScriptObject", "Failed to create new object");
			release();
		}

		return *this;
	}

	ScriptObject::ScriptObject(const ScriptVariableBase& variable) : ScriptObject()
	{
		if (variable.is_object() || variable.is_handle())
		{
			if (!create(variable.address(), variable.type_id(), true))
			{
				error_log("ScriptObject", "Failed to convert script variable to script object!");
			}
		}
	}

	ScriptObject::ScriptObject(ScriptVariableBase&& variable)
	{
		if (variable.is_object() || variable.is_handle())
		{
			ScriptVariableBase::operator=(std::move(variable));
		}
	}

	ScriptObject& ScriptObject::operator=(const ScriptVariableBase& variable)
	{
		if (this == &variable)
			return *this;

		if (variable.is_object() || variable.is_handle())
		{
			if (!create(variable.address(), variable.type_id(), true))
			{
				error_log("ScriptObject", "Failed to convert script variable to script object!");
				release();
			}
		}
		return *this;
	}

	ScriptObject& ScriptObject::operator=(ScriptVariableBase&& variable)
	{
		if (variable.is_object() || variable.is_handle())
		{
			ScriptVariableBase::operator=(std::move(variable));
		}
		return *this;
	}

	bool ScriptObject::create(int_t type_id, bool is_uninitialized)
	{
		if (!ScriptEngine::is_object_type(type_id))
		{
			error_log("ScriptObject", "Type ID is not refer to object type!");
			return false;
		}

		release();
		m_type_id = type_id;

		asIScriptEngine* engine = ScriptEngine::engine();

		if (is_uninitialized)
		{
			m_address = engine->CreateUninitializedScriptObject(engine->GetTypeInfoById(type_id));
		}
		else
		{
			m_address = engine->CreateScriptObject(engine->GetTypeInfoById(type_id));
		}

		if (m_address == nullptr)
		{
			release();
			error_log("ScriptObject", "Failed to create script object!");
			return false;
		}

		add_ref();
		return true;
	}

	bool ScriptObject::create(const char* declaration, bool is_uninitialized)
	{
		return create(find_type_id(declaration), is_uninitialized);
	}

	bool ScriptObject::create(const char* type_declaration, const char* module_name, bool is_uninitialized)
	{
		return create(find_type_id(type_declaration, module_name), is_uninitialized);
	}

	bool ScriptObject::create(void* src_address, int_t type_id, bool consider_handle_as_object)
	{
		if (!ScriptEngine::is_object_type(type_id, true))
		{
			error_log("ScriptObject", "Type ID is not refer to object or handle type!");
			return false;
		}

		release();
		m_type_id = type_id;

		if (is_object())
		{
			asIScriptEngine* engine = ScriptEngine::engine();
			m_address				= engine->CreateScriptObjectCopy(src_address, engine->GetTypeInfoById(type_id));

			if (m_address == nullptr)
			{
				release();
				error_log("ScriptVariableBase", "Failed to create script object!");
				return false;
			}
		}
		else if (is_handle())
		{
			m_address = object_from_handle(src_address, consider_handle_as_object);
		}

		add_ref();
		return true;
	}

	bool ScriptObject::create(void* src_address, const char* type_declaration, bool consider_handle_as_object)
	{
		return create(src_address, find_type_id(type_declaration), consider_handle_as_object);
	}

	bool ScriptObject::create(void* src_address, const char* type_declaration, const char* module, bool consider_handle_as_object)
	{
		return create(src_address, find_type_id(type_declaration, module), consider_handle_as_object);
	}

	uint_t ScriptObject::factory_count() const
	{
		return type_info().factory_count();
	}

	ScriptFunction ScriptObject::factory_by_index(uint_t index) const
	{
		return type_info().factory_by_index(index);
	}

	ScriptFunction ScriptObject::factory_by_decl(const char* decl) const
	{
		return type_info().factory_by_decl(decl);
	}

	ScriptFunction ScriptObject::factory_by_decl(const String& decl) const
	{
		return type_info().factory_by_decl(decl);
	}

	// Methods
	uint_t ScriptObject::method_count() const
	{
		return type_info().method_count();
	}

	ScriptFunction ScriptObject::method_by_index(uint_t index, bool get_virtual) const
	{
		return type_info().method_by_index(index, get_virtual);
	}

	ScriptFunction ScriptObject::method_by_name(const char* name, bool get_virtual) const
	{
		return type_info().method_by_name(name, get_virtual);
	}

	ScriptFunction ScriptObject::method_by_decl(const char* decl, bool get_virtual) const
	{
		return type_info().method_by_decl(decl, get_virtual);
	}

	ScriptFunction ScriptObject::method_by_name(const String& name, bool get_virtual) const
	{
		return type_info().method_by_name(name, get_virtual);
	}

	ScriptFunction ScriptObject::method_by_decl(const String& decl, bool get_virtual) const
	{
		return type_info().method_by_decl(decl, get_virtual);
	}

	// Properties
	uint_t ScriptObject::property_count() const
	{
		return type_info().property_count();
	}

	bool ScriptObject::property(uint_t index, StringView* name, int_t* type_id, bool* is_private, bool* is_protected,
								int_t* offset, bool* is_reference) const
	{
		return type_info().property(index, name, type_id, is_private, is_protected, offset, is_reference);
	}

	String ScriptObject::property_declaration(uint_t index, bool include_bamespace) const
	{
		return type_info().property_declaration(index, include_bamespace);
	}

	// Behaviours
	uint_t ScriptObject::behaviour_count() const
	{
		return type_info().behaviour_count();
	}

	ScriptFunction ScriptObject::behaviour_by_index(uint_t index, ScriptClassBehave* behaviour) const
	{
		return type_info().behaviour_by_index(index, behaviour);
	}

	ScriptTypeInfo ScriptObject::type_info() const
	{
		if (m_info.type_id() != type_id())
		{
			m_info = ScriptVariableBase::type_info();
		}
		return m_info;
	}
}// namespace Engine
