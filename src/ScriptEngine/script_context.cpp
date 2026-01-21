#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/templates.hpp>
#include <Core/logger.hpp>
#include <Core/math/vector.hpp>
#include <ScriptEngine/script.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <ScriptEngine/script_module.hpp>
#include <ScriptEngine/script_object.hpp>
#include <ScriptEngine/script_primitives.hpp>
#include <angelscript.h>
#include <scriptdictionary.h>
#include <scripthelper.h>

namespace Engine
{
	static asIScriptContext* m_context      = nullptr;
	static Function<void(void*)> m_callback = {};

	static void script_line_callback_internal(asIScriptEngine* engine, void* userdata)
	{
		if (m_callback)
			m_callback(userdata);
	}

	static void script_exception_callback(asIScriptContext* ctx, void* object)
	{
		ScriptFunction function      = ctx->GetExceptionFunction();
		const char* exception_string = ctx->GetExceptionString();
		int_t column                 = 0;
		const int_t line             = ctx->GetExceptionLineNumber(&column);

		Script* script = function.module().script();

		if (script)
		{
			error_log("ScriptEngine", "Script Exception: %s (Line: %d, Column: %d): %s", script->path().c_str(), line, column,
			          exception_string);
			script->on_exception(script);
		}
	}

	void ScriptContext::initialize()
	{
		m_context = ScriptEngine::engine()->RequestContext();
		m_context->AddRef();

		m_context->SetExceptionCallback(asFUNCTION(script_exception_callback), nullptr, asCALL_CDECL);
	}

	void ScriptContext::terminate()
	{
		clear_line_callback();
		m_context->Release();
		ScriptEngine::engine()->ReturnContext(m_context);
		m_context = nullptr;
	}

	ScriptContext& ScriptContext::instance()
	{
		static ScriptContext context;
		return context;
	}

	bool ScriptContext::begin_execute(const ScriptFunction& function)
	{
		auto current_state = state();

		if (!is_in<State::Uninitialized, State::Active>(current_state))
		{
			if (current_state == State::Exception)
			{
				throw EngineException(exception_string());
			}
			throw EngineException("State of context must be Uninitialized or Active!");
		}

		if (current_state == State::Active)
		{
			if (!push_state())
			{
				throw EngineException("Failed to push new state!");
			}
		}

		if (!prepare(function))
		{
			if (current_state == State::Active)
			{
				pop_state();
			}

			throw EngineException("Failed to prepare function!");
		}

		return true;
	}

	bool ScriptContext::end_execute(void* return_value)
	{
		const bool is_active = callstack_size() > 1;

		bool is_prepared = state() == State::Prepared;

		if (is_prepared)
		{
			if (!execute())
			{
				unprepare();

				if (is_active)
				{
					pop_state();
				}

				throw EngineException("Failed to execute script function!");
			}

			ScriptFunction current_function = function();

			ScriptTypeModifiers modifiers;
			auto type_id = current_function.return_type_id(&modifiers);

			if (type_id != 0 && return_value)
			{
				if ((modifiers & ScriptTypeModifiers::OutRef) || ScriptEngine::is_handle_type(type_id))
				{
					(*static_cast<void**>(return_value)) = return_address();
				}
				else if (ScriptEngine::is_primitive_type(type_id))
				{
					std::memcpy(return_value, address_of_return_value(), ScriptEngine::sizeof_primitive_type(type_id));
				}
				else
				{
					ScriptEngine::assign_script_object(return_value, return_object_ptr(), ScriptEngine::type_info_by_id(type_id));
				}
			}
		}

		if (!unprepare())
			throw EngineException("Failed to unprepare function!");

		if (is_active)
		{
			if (!pop_state())
				throw EngineException("Failed to pop state!");
		}

		return is_prepared;
	}

	asIScriptContext* ScriptContext::context()
	{
		return m_context;
	}

	bool ScriptContext::prepare(const ScriptFunction& func)
	{
		return m_context->Prepare(func.function()) >= 0;
	}

	bool ScriptContext::unprepare()
	{
		return m_context->Unprepare() >= 0;
	}

	bool ScriptContext::execute()
	{
		return m_context->Execute() >= 0;
	}

	bool ScriptContext::abort()
	{
		return m_context->Abort() >= 0;
	}

	bool ScriptContext::suspend()
	{
		return m_context->Suspend() >= 0;
	}

	ScriptContext::State ScriptContext::state()
	{
		auto state = m_context->GetState();
		switch (state)
		{
			case asEXECUTION_FINISHED: return State::Finished;
			case asEXECUTION_SUSPENDED: return State::Suspended;
			case asEXECUTION_ABORTED: return State::Aborted;
			case asEXECUTION_EXCEPTION: return State::Exception;
			case asEXECUTION_PREPARED: return State::Prepared;
			case asEXECUTION_UNINITIALIZED: return State::Uninitialized;
			case asEXECUTION_ACTIVE: return State::Active;
			case asEXECUTION_ERROR: return State::Error;
			case asEXECUTION_DESERIALIZATION: return State::Deserealization;
			default: return State::Undefined;
		}
	}

	bool ScriptContext::push_state()
	{
		return m_context->PushState() >= 0;
	}

	bool ScriptContext::pop_state()
	{
		return m_context->PopState() >= 0;
	}

	uint_t ScriptContext::nest_count()
	{
		asUINT count = 0;
		if (m_context->IsNested(&count))
		{
			return static_cast<uint_t>(count);
		}
		return 0;
	}

	bool ScriptContext::object(const ScriptObject& object)
	{
		return ScriptContext::object(object.address());
	}

	bool ScriptContext::object(const void* address)
	{
		if (address == nullptr)
			return false;
		return m_context->SetObject(const_cast<void*>(address)) >= 0;
	}

	bool ScriptContext::arg_bool(uint_t arg, bool value)
	{
		return m_context->SetArgByte(arg, value) >= 0;
	}

	bool ScriptContext::arg_byte(uint_t arg, byte value)
	{
		return m_context->SetArgByte(arg, value) >= 0;
	}

	bool ScriptContext::arg_word(uint_t arg, word value)
	{
		return m_context->SetArgWord(arg, value) >= 0;
	}

	bool ScriptContext::arg_dword(uint_t arg, dword value)
	{
		return m_context->SetArgDWord(arg, value) >= 0;
	}

	bool ScriptContext::arg_qword(uint_t arg, qword value)
	{
		return m_context->SetArgQWord(arg, value) >= 0;
	}

	bool ScriptContext::arg_float(uint_t arg, float value)
	{
		return m_context->SetArgFloat(arg, value) >= 0;
	}

	bool ScriptContext::arg_double(uint_t arg, double value)
	{
		return m_context->SetArgDouble(arg, value) >= 0;
	}

	bool ScriptContext::arg_script_obj(uint_t arg, const ScriptObject& obj)
	{
		return m_context->SetArgObject(arg, obj.address()) >= 0;
	}

	bool ScriptContext::arg_var_type(uint_t arg, void* ptr, int_t type_id)
	{
		return m_context->SetArgVarType(arg, ptr, type_id);
	}

	bool ScriptContext::arg_address(uint_t arg, void* addr, bool is_object)
	{
		if (is_object)
			return m_context->SetArgObject(arg, addr) >= 0;

		return m_context->SetArgAddress(arg, addr) >= 0;
	}

	void* ScriptContext::address_of_arg(uint_t arg)
	{
		return m_context->GetAddressOfArg(arg);
	}

	uint8_t ScriptContext::return_byte()
	{
		return static_cast<uint8_t>(m_context->GetReturnByte());
	}

	uint16_t ScriptContext::return_word()
	{
		return static_cast<uint16_t>(m_context->GetReturnWord());
	}

	uint32_t ScriptContext::return_dword()
	{
		return static_cast<uint32_t>(m_context->GetReturnDWord());
	}

	uint64_t ScriptContext::return_qword()
	{
		return static_cast<uint64_t>(m_context->GetReturnQWord());
	}

	float ScriptContext::return_float()
	{
		return m_context->GetReturnFloat();
	}

	double ScriptContext::return_double()
	{
		return m_context->GetReturnDouble();
	}

	void* ScriptContext::return_address()
	{
		return m_context->GetReturnAddress();
	}

	void* ScriptContext::return_object_ptr()
	{
		int_t return_typeid = function(0).return_type_id();
		if (return_typeid & asTYPEID_MASK_OBJECT)
		{
			return m_context->GetReturnObject();
		}
		return nullptr;
	}

	ScriptObject ScriptContext::return_object()
	{
		int_t return_typeid = function(0).return_type_id();
		if (return_typeid & asTYPEID_MASK_OBJECT)
		{
			return ScriptObject(m_context->GetReturnObject(), return_typeid);
		}
		return {};
	}

	void* ScriptContext::address_of_return_value()
	{
		return m_context->GetAddressOfReturnValue();
	}

	bool ScriptContext::exception(const char* info, bool allow_catch)
	{
		return m_context->SetException(info, allow_catch) >= 0;
	}

	bool ScriptContext::exception(const String& info, bool allow_catch)
	{
		return exception(info.c_str(), allow_catch);
	}

	Vector2i ScriptContext::exception_line_position(StringView* section)
	{
		Vector2i result  = {-1, -1};
		const char* name = nullptr;
		result.y         = m_context->GetExceptionLineNumber(&result.x, section ? &name : nullptr);

		if (section && name)
		{
			(*section) = name;
		}
		return result;
	}

	ScriptFunction ScriptContext::exception_function()
	{
		return ScriptFunction(m_context->GetExceptionFunction());
	}

	String ScriptContext::exception_string()
	{
		if (const char* text = m_context->GetExceptionString())
		{
			return text;
		}
		return "";
	}

	bool ScriptContext::will_exception_be_caught()
	{
		return m_context->WillExceptionBeCaught();
	}

	bool ScriptContext::line_callback(const Function<void(void*)>& function, void* userdata)
	{
		m_callback        = function;
		const bool result = m_context->SetLineCallback(asFUNCTION(script_line_callback_internal), userdata, asCALL_CDECL) >= 0;
		if (!result)
			clear_line_callback();
		return result;
	}

	bool ScriptContext::line_callback(const ScriptFunction& function)
	{
		return line_callback(
		        [function](void*) {
			        m_context->ClearLineCallback();
			        ScriptContext::execute(function);
			        m_context->SetLineCallback(asFUNCTION(script_line_callback_internal), nullptr, asCALL_CDECL);
		        },
		        nullptr);
	}

	ScriptContext& ScriptContext::clear_line_callback()
	{
		Function<void(void*)> tmp = {};
		m_callback.swap(tmp);
		m_context->ClearLineCallback();
		return instance();
	}

	uint_t ScriptContext::callstack_size()
	{
		return m_context->GetCallstackSize();
	}

	ScriptFunction ScriptContext::function(uint_t stack_level)
	{
		return ScriptFunction(m_context->GetFunction(stack_level));
	}

	ScriptFunction ScriptContext::system_function()
	{
		return ScriptFunction(m_context->GetSystemFunction());
	}

	Vector2i ScriptContext::line_position(uint_t stack_level, StringView* section_name)
	{
		Vector2i result     = {-1, -1};
		const char* section = nullptr;
		result.y            = m_context->GetLineNumber(stack_level, &result.x, (section_name ? &section : nullptr));

		if (section_name)
		{
			(*section_name) = section ? section : "";
		}

		return result;
	}

	uint_t ScriptContext::var_count(uint_t stack_level)
	{
		const int count = m_context->GetVarCount(stack_level);
		return count > 0 ? static_cast<uint_t>(count) : 0;
	}

	bool ScriptContext::var(uint_t var_index, uint_t stack_level, StringView* name, int_t* type_id,
	                        ScriptTypeModifiers* modifiers, bool* is_var_on_heap, int_t* stack_offset)
	{
		asETypeModifiers script_modifiers;
		const char* script_name;
		const bool result = m_context->GetVar(var_index, stack_level, &script_name, type_id,
		                                      (modifiers ? &script_modifiers : nullptr), is_var_on_heap, stack_offset) >= 0;

		if (name)
		{
			(*name) = script_name ? script_name : "";
		}

		if (modifiers)
		{
			(*modifiers) = Flags<ScriptTypeModifiers>(static_cast<BitMask>(script_modifiers));
		}
		return result;
	}

	String ScriptContext::var_declaration(uint_t var_index, uint_t stack_level, bool include_namespace)
	{
		if (auto decl = m_context->GetVarDeclaration(var_index, stack_level, include_namespace))
			return decl;
		return "";
	}

	byte* ScriptContext::address_of_var(uint_t var_index, uint_t stack_level, bool dont_dereference,
	                                    bool return_address_of_unitialized_objects)
	{
		return reinterpret_cast<byte*>(
		        m_context->GetAddressOfVar(var_index, stack_level, dont_dereference, return_address_of_unitialized_objects));
	}

	bool ScriptContext::is_var_in_scope(uint_t var_index, uint_t stack_level)
	{
		return m_context->IsVarInScope(var_index, stack_level);
	}

	int_t ScriptContext::this_type_id(uint_t stack_level)
	{
		return m_context->GetThisTypeId(stack_level);
	}

	byte* ScriptContext::this_pointer(uint_t stack_level)
	{
		return reinterpret_cast<byte*>(m_context->GetThisPointer(stack_level));
	}
}// namespace Engine
