#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

namespace Engine
{
	static FORCE_INLINE void* object_from_handle(void* address, bool handle_is_object)
	{
		if (address == nullptr)
			return address;

		if (handle_is_object)
			return address;

		return *reinterpret_cast<void**>(address);
	}

	ScriptVariableBase::ScriptVariableBase() : m_address(nullptr), m_type_id(0) {}

	ScriptVariableBase::ScriptVariableBase(const ScriptTypeInfo& info, bool is_uninitialized) : ScriptVariableBase()
	{
		if (!create(info, is_uninitialized))
		{
			throw EngineException("Failed to create new script variable!");
		}
	}

	ScriptVariableBase::ScriptVariableBase(void* src_address, const ScriptTypeInfo& info, bool handle_is_object,
										   const ScriptTypeModifiers& modifiers)
	    : ScriptVariableBase()
	{
		if (!create(src_address, info, handle_is_object, modifiers))
		{
			throw EngineException("Failed to create new script variable!");
		}
	}

	ScriptVariableBase::ScriptVariableBase(const ScriptVariableBase& object) : ScriptVariableBase() {}

	ScriptVariableBase::ScriptVariableBase(ScriptVariableBase&& object) : ScriptVariableBase()
	{
		(*this) = std::move(object);
	}

	ScriptVariableBase& ScriptVariableBase::operator=(ScriptVariableBase&& object)
	{
		if (this == &object)
			return *this;

		release();
		m_address   = object.m_address;
		m_type_id   = object.m_type_id;
		m_modifiers = object.m_modifiers;

		object.m_address   = nullptr;
		object.m_type_id   = 0;
		object.m_modifiers = {};
		return *this;
	}

	ScriptVariableBase& ScriptVariableBase::operator=(const ScriptVariableBase& object)
	{
		return *this;
	}

	bool ScriptVariableBase::operator==(const ScriptVariableBase& other) const
	{
		return m_address == other.m_address && m_type_id == other.m_type_id;
	}

	bool ScriptVariableBase::operator!=(const ScriptVariableBase& other) const
	{
		return m_address != other.m_address && m_type_id != other.m_type_id;
	}

	const ScriptVariableBase& ScriptVariableBase::add_ref() const
	{
		if (!is_valid())
			return *this;

		if (is_handle() && m_address)
		{
			auto info = type_info().info();

			if (!(info->GetFlags() & asOBJ_NOCOUNT))
			{
				asIScriptEngine* engine = ScriptEngine::engine();
				engine->AddRefScriptObject(address(), info);
			}
		}

		return *this;
	}

	const ScriptVariableBase& ScriptVariableBase::release() const
	{
		if (!is_valid())
			return *this;

		if ((is_handle() || (is_object() && !is_out_ref())) && m_address)
		{
			auto info = type_info().info();

			if (!(info->GetFlags() & asOBJ_NOCOUNT))
			{
				asIScriptEngine* engine = ScriptEngine::engine();
				engine->ReleaseScriptObject(address(), info);
			}
		}

		m_address   = nullptr;
		m_type_id   = 0;
		m_modifiers = {};
		return *this;
	}

	bool ScriptVariableBase::assign(const ScriptVariableBase& other)
	{
		if (this == &other)
			return true;
		return assign(other.address(), true);
	}

	bool ScriptVariableBase::assign(void* var_address, bool handle_is_object)
	{
		if (var_address == nullptr && !is_handle())
		{
			error_log("ScriptVariableBase", "Cannot assing invalid object!");
			return false;
		}

		if (is_handle())
		{
			asIScriptEngine* engine = ScriptEngine::engine();

			auto info = type_info();
			if (m_address)
			{
				engine->ReleaseScriptObject(m_address, info.info());
			}

			m_address = object_from_handle(var_address, handle_is_object);

			if (m_address)
			{
				engine->AddRefScriptObject(m_address, info.info());
			}
		}
		else if (is_object())
		{
			if (m_address == nullptr)
			{
				error_log("ScriptVariableBase", "Cannot assing object: Current object is invalid!");
				return false;
			}

			asIScriptEngine* engine = ScriptEngine::engine();

			if (engine->AssignScriptObject(m_address, var_address, type_info().info()) < 0)
			{
				error_log("ScriptVariableBase", "Failed to assing new object");
				return false;
			}
		}
		else
		{
			int_t size = ScriptEngine::sizeof_primitive_type(type_id());

			if (size > 0)
			{
				std::memcpy(m_address, var_address, size);
			}
		}

		return true;
	}

	bool ScriptVariableBase::assign_to(ScriptVariableBase& other) const
	{
		return other.assign(*this);
	}

	bool ScriptVariableBase::assign_to(void* dst) const
	{
		if (is_handle())
		{
			(*reinterpret_cast<void**>(dst)) = address();
			add_ref();
			return true;
		}
		else if (is_object())
		{
			asIScriptEngine* engine = ScriptEngine::engine();
			return engine->AssignScriptObject(dst, address(), type_info().info()) >= 0;
		}
		else
		{
			asIScriptEngine* engine = ScriptEngine::engine();
			size_t size             = engine->GetSizeOfPrimitiveType(type_id());
			if (size > 0)
			{
				std::memcpy(dst, address(), size);
				return true;
			}
			return false;
		}

		return false;
	}

	bool ScriptVariableBase::create(const ScriptTypeInfo& info, bool is_uninitialized)
	{
		release();
		m_type_id = info.type_id();

		if (is_object())
		{
			asIScriptEngine* engine = ScriptEngine::engine();

			if (is_uninitialized)
			{
				m_address = engine->CreateUninitializedScriptObject(info.info());
			}
			else
			{
				m_address = engine->CreateScriptObject(info.info());
			}

			if (m_address == nullptr)
			{
				release();
				error_log("ScriptVariableBase", "Failed to create script object!");
				return false;
			}
		}

		add_ref();
		return true;
	}

	bool ScriptVariableBase::create(void* address, const ScriptTypeInfo& info, bool handle_is_object,
									const ScriptTypeModifiers& modifiers)
	{
		release();
		m_type_id   = info.type_id();
		m_modifiers = modifiers;

		if (is_handle())
		{
			m_address = object_from_handle(address, handle_is_object);
		}
		else if (is_object())
		{
			asIScriptEngine* engine = ScriptEngine::engine();
			if (is_out_ref())
			{
				m_address = address;
			}
			else
			{
				m_address = engine->CreateScriptObjectCopy(address, info.info());
			}

			if (m_address == nullptr)
			{
				release();
				error_log("ScriptVariableBase", "Failed to create script object!");
				return false;
			}
		}

		add_ref();
		return true;
	}

	bool ScriptVariableBase::is_valid() const
	{
		return m_type_id != 0;
	}

	bool ScriptVariableBase::check_type(int_t mask) const
	{
		return m_type_id == mask;
	}

	bool ScriptVariableBase::is_object(bool handle_is_object) const
	{
		return ScriptEngine::is_object_type(m_type_id, handle_is_object);
	}

	bool ScriptVariableBase::is_handle() const
	{
		return ScriptEngine::is_handle_type(m_type_id);
	}

	bool ScriptVariableBase::is_const() const
	{
		return m_modifiers & ScriptTypeModifiers::Const;
	}

	bool ScriptVariableBase::is_in_ref() const
	{
		return m_modifiers & ScriptTypeModifiers::InRef;
	}

	bool ScriptVariableBase::is_out_ref() const
	{
		return m_modifiers & ScriptTypeModifiers::OutRef;
	}

	bool ScriptVariableBase::is_inout_ref() const
	{
		return (m_modifiers & ScriptTypeModifiers::InOutRef) == ScriptTypeModifiers::InOutRef;
	}

	void* ScriptVariableBase::address() const
	{
		if (!is_valid())
			return nullptr;
		if (is_object(true))
			return m_address;
		return &m_address;
	}

	int_t ScriptVariableBase::type_id() const
	{
		return m_type_id;
	}

	ScriptTypeInfo ScriptVariableBase::type_info() const
	{
		return ScriptEngine::type_info_by_id(m_type_id);
	}

	ScriptVariableBase::~ScriptVariableBase()
	{
		release();
	}

	ScriptVariable::ScriptVariable(int_t type_id)
	{
		if (!create(type_id))
		{
			throw EngineException("Failed to create new script variable!");
		}
	}

	ScriptVariable::ScriptVariable(const char* declaration) : ScriptVariable(ScriptEngine::type_id_by_decl(declaration)) {}

	static int_t find_type_id_internal(const char* declaration, const char* module_name)
	{
		ScriptModule module(module_name, ScriptModule::OnlyIfExists);
		if (module.is_valid())
		{
			return module.type_id_by_decl(declaration);
		}
		return 0;
	}

	ScriptVariable::ScriptVariable(const char* declaration, const char* module)
	    : ScriptVariable(find_type_id_internal(declaration, module))
	{}

	ScriptVariable::ScriptVariable(void* address, int_t type_id, bool handle_is_object, const ScriptTypeModifiers& modifiers)
	{
		if (!create(address, type_id, handle_is_object, modifiers))
		{
			throw EngineException("Failed to create new script variable!");
		}
	}

	ScriptVariable::ScriptVariable(void* address, const char* declaration, bool handle_is_object,
								   const ScriptTypeModifiers& modifiers)
	    : ScriptVariable(address, ScriptEngine::type_id_by_decl(declaration), handle_is_object, modifiers)
	{}

	ScriptVariable::ScriptVariable(void* address, const char* declaration, const char* module, bool handle_is_object,
								   const ScriptTypeModifiers& modifiers)
	    : ScriptVariable(address, find_type_id_internal(declaration, module), handle_is_object, modifiers)
	{}

	ScriptVariable::ScriptVariable(const ScriptVariable& object)
	    : ScriptVariable(object.address(), object.type_id(), true, object.m_modifiers)
	{}

	ScriptVariable::ScriptVariable(ScriptVariable&& object) : ScriptVariableBase(std::move(object)) {}

	ScriptVariable& ScriptVariable::operator=(ScriptVariable&& object)
	{
		ScriptVariableBase::operator=(std::move(object));
		return *this;
	}

	ScriptVariable& ScriptVariable::operator=(const ScriptVariable& object)
	{
		if (this == &object)
			return *this;

		if (!create(object))
		{
			throw EngineException("Failed to create new script variable!");
		}

		return *this;
	}

	bool ScriptVariable::create(int_t type_id, bool is_uninitialized)
	{
		release();
		m_type_id = type_id;

		if (is_object())
		{
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
				error_log("ScriptVariableBase", "Failed to create script object!");
				return false;
			}
		}

		add_ref();
		return true;
	}

	bool ScriptVariable::create(void* address, int_t type_id, bool handle_is_object, const ScriptTypeModifiers& modifiers)
	{
		release();
		m_type_id   = type_id;
		m_modifiers = modifiers;

		if (is_handle())
		{
			m_address = object_from_handle(address, handle_is_object);
		}
		else if (is_object())
		{
			asIScriptEngine* engine = ScriptEngine::engine();

			if (is_out_ref())
			{
				m_address = address;
			}
			else
			{
				m_address = engine->CreateScriptObjectCopy(address, engine->GetTypeInfoById(type_id));
			}

			if (m_address == nullptr)
			{
				release();
				error_log("ScriptVariableBase", "Failed to create script object!");
				return false;
			}
		}
		else
		{
			int_t size = ScriptEngine::sizeof_primitive_type(type_id);
			if (size > 0)
			{
				std::memcpy(&m_address, address, size);
			}
		}

		add_ref();
		return true;
	}

	bool ScriptVariable::create(void* src_address, const char* type_declaration, bool handle_is_object,
								const ScriptTypeModifiers& modifiers)
	{
		int_t type_id = ScriptEngine::type_id_by_decl(type_declaration);
		if (type_id < 0)
		{
			error_log("ScriptVariableBase", "Cannot create script variable, because type_id is invalid!");
			return false;
		}
		return create(src_address, type_id, handle_is_object, modifiers);
	}

	bool ScriptVariable::create(void* src_address, const char* type_declaration, const char* module, bool handle_is_object,
								const ScriptTypeModifiers& modifiers)
	{
		int_t type_id = find_type_id_internal(type_declaration, module);
		if (type_id < 0)
		{
			error_log("ScriptVariableBase", "Cannot create script variable, because type_id is invalid!");
			return false;
		}
		return create(src_address, type_id, handle_is_object, modifiers);
	}

	bool ScriptVariable::create(const ScriptVariableBase& other)
	{
		return create(other.address(), other.type_id(), true);
	}

	bool ScriptVariable::create(const char* type_declaration, bool is_uninitialized)
	{
		int_t type_id = ScriptEngine::type_id_by_decl(type_declaration);
		if (type_id < 0)
		{
			error_log("ScriptVariableBase", "Cannot create script variable, because type_id is invalid!");
			return false;
		}
		return create(type_id, is_uninitialized);
	}

	bool ScriptVariable::create(const char* type_declaration, const char* module, bool is_uninitialized)
	{
		int_t type_id = find_type_id_internal(type_declaration, module);
		if (type_id < 0)
		{
			error_log("ScriptVariableBase", "Cannot create script variable, because type_id is invalid!");
			return false;
		}
		return create(type_id, is_uninitialized);
	}

	bool ScriptVariable::is_bool() const
	{
		return check_type(asTYPEID_BOOL);
	}

	bool ScriptVariable::is_int8() const
	{
		return check_type(asTYPEID_INT8);
	}

	bool ScriptVariable::is_int16() const
	{
		return check_type(asTYPEID_INT16);
	}

	bool ScriptVariable::is_int32() const
	{
		return check_type(asTYPEID_INT32);
	}

	bool ScriptVariable::is_int64() const
	{
		return check_type(asTYPEID_INT64);
	}

	bool ScriptVariable::is_uint8() const
	{
		return check_type(asTYPEID_UINT8);
	}

	bool ScriptVariable::is_uint16() const
	{
		return check_type(asTYPEID_UINT16);
	}

	bool ScriptVariable::is_uint32() const
	{
		return check_type(asTYPEID_UINT32);
	}

	bool ScriptVariable::is_uint64() const
	{
		return check_type(asTYPEID_UINT64);
	}

	bool ScriptVariable::is_float() const
	{
		return check_type(asTYPEID_FLOAT);
	}

	bool ScriptVariable::is_double() const
	{
		return check_type(asTYPEID_DOUBLE);
	}

	bool ScriptVariable::bool_value() const
	{
		return m_bool_value;
	}

	int8_t ScriptVariable::int8_value() const
	{
		return m_int8_value;
	}

	int16_t ScriptVariable::int16_value() const
	{
		return m_int16_value;
	}

	int32_t ScriptVariable::int32_value() const
	{
		return m_int32_value;
	}

	int64_t ScriptVariable::int64_value() const
	{
		return m_int64_value;
	}

	uint8_t ScriptVariable::uint8_value() const
	{
		return m_uint8_value;
	}

	uint16_t ScriptVariable::uint16_value() const
	{
		return m_uint16_value;
	}

	uint32_t ScriptVariable::uint32_value() const
	{
		return m_uint32_value;
	}

	uint64_t ScriptVariable::uint64_value() const
	{
		return m_uint64_value;
	}

	float ScriptVariable::float_value() const
	{
		return m_float_value;
	}

	double ScriptVariable::double_value() const
	{
		return m_double_value;
	}
}// namespace Engine
