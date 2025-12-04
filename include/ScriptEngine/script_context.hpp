#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/ref.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/flags.hpp>
#include <Core/math/fwd.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

class asIScriptContext;
class asIScriptFunction;

namespace Engine
{
	class ScriptFunction;
	class ScriptObject;

	class ENGINE_EXPORT ScriptContext
	{
	private:
		static void initialize();
		static void terminate();

		struct ExecutionInfo {
			ScriptTypeModifiers return_type_modifiers;
			int_t return_type_id = 0;
			bool is_active       = false;
		};

	public:
		enum class State
		{
			Undefined       = 0,
			Finished        = 1,
			Suspended       = 2,
			Aborted         = 3,
			Exception       = 4,
			Prepared        = 5,
			Uninitialized   = 6,
			Active          = 7,
			Error           = 8,
			Deserealization = 9,
		};

		static ScriptContext& instance();
		static bool begin_execute(const ScriptFunction& function);
		static bool end_execute(void* return_value = nullptr);

		static asIScriptContext* context();
		static bool prepare(const ScriptFunction& func);
		static bool unprepare();
		static bool execute();
		static bool abort();
		static bool suspend();
		static State state();
		static bool push_state();
		static bool pop_state();
		static uint_t nest_count();

		static bool object(const ScriptObject& object);
		static bool object(const void* address);

		static bool arg_bool(uint_t arg, bool value);
		static bool arg_byte(uint_t arg, byte value);
		static bool arg_word(uint_t arg, word value);
		static bool arg_dword(uint_t arg, dword value);
		static bool arg_qword(uint_t arg, qword value);
		static bool arg_float(uint_t arg, float value);
		static bool arg_double(uint_t arg, double value);
		static bool arg_script_obj(uint_t arg, const ScriptObject& obj);
		static bool arg_address(uint_t arg, void* addr, bool is_object = false);
		static bool arg_var_type(uint_t arg, void* ptr, int_t type_id);

		template<typename ValueType>
		static bool arg(uint_t idx, ValueType&& value)
		{
			using T = std::decay_t<ValueType>;

			if constexpr (std::is_integral_v<T>)
			{
				if constexpr (std::is_same_v<T, bool>)
					return arg_bool(idx, value);
				else if constexpr (sizeof(T) == sizeof(byte))
					return arg_bool(idx, static_cast<byte>(value));
				else if constexpr (sizeof(T) == sizeof(word))
					return arg_word(idx, static_cast<word>(value));
				else if constexpr (sizeof(T) == sizeof(dword))
					return arg_dword(idx, static_cast<dword>(value));
				else if constexpr (sizeof(T) == sizeof(qword))
					return arg_qword(idx, static_cast<qword>(value));
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				if constexpr (std::is_same_v<T, float>)
					return arg_float(idx, value);
				else
					return arg_double(idx, static_cast<double>(value));
			}
			else if constexpr (std::is_pointer_v<T>)
			{
				using BaseType = std::decay_t<std::remove_pointer_t<T>>*;
				return arg_address(idx, static_cast<void*>(const_cast<BaseType>(value)), false);
			}

			return false;
		}

		template<typename T>
		static bool arg(uint_t idx, RRef<T>& ref)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return arg_address(idx, ref.get(), false);
			}
			else
			{
				return arg_address(idx, ref.address(), false);
			}
		}

		template<typename T>
		static bool arg(uint_t idx, LRef<T> ref)
		{
			if constexpr (std::is_pointer_v<T>)
			{
				return arg_address(idx, ref.get(), false);
			}
			else
			{
				return arg_address(idx, ref.address(), false);
			}
		}

		static void* address_of_arg(uint_t arg);

		// Return value
		static uint8_t return_byte();
		static uint16_t return_word();
		static uint32_t return_dword();
		static uint64_t return_qword();
		static float return_float();
		static double return_double();
		static void* return_address();
		static ScriptObject return_object();
		static void* return_object_ptr();
		static void* address_of_return_value();

		// Function execution
		template<typename... Args>
		static inline bool execute(const ScriptFunction& function, void* return_value = nullptr, const Args&... args)
		{
			begin_execute(function);

			uint_t argument = 0;
			(arg(argument++, args), ...);

			return end_execute(return_value);
		}

		template<typename... Args>
		static inline bool execute(const ScriptObject& self, const ScriptFunction& function, void* return_value = nullptr,
		                           const Args&... args)
		{
			begin_execute(function);

			uint_t argument = 0;
			object(self), (arg(argument++, args), ...);

			return end_execute(return_value);
		}

		template<typename... Args>
		static inline bool execute(const void* self, const ScriptFunction& function, void* return_value = nullptr,
		                           const Args&... args)
		{
			begin_execute(function);

			uint_t argument = 0;
			object(self), (arg(argument++, args), ...);

			return end_execute(return_value);
		}

		// Exception handling
		static bool exception(const char* info, bool allow_catch = true);
		static bool exception(const String& info, bool allow_catch = true);
		static Vector2i exception_line_position(StringView* section_name = nullptr);
		static ScriptFunction exception_function();
		static String exception_string();
		static bool will_exception_be_caught();

		// Debugging
		static bool line_callback(const Function<void(void*)>& function, void* userdata = nullptr);
		static bool line_callback(const ScriptFunction& function);
		static ScriptContext& clear_line_callback();

		static uint_t callstack_size();
		static ScriptFunction function(uint_t stack_level = 0);
		static Vector2i line_position(uint_t stack_level = 0, StringView* section_name = nullptr);
		static uint_t var_count(uint_t stack_level = 0);
		static bool var(uint_t var_index, uint_t stack_level, StringView* name, int_t* type_id = 0,
		                ScriptTypeModifiers* modifiers = nullptr, bool* is_var_on_heap = 0, int_t* stack_offset = 0);
		static String var_declaration(uint_t var_index, uint_t stack_level = 0, bool include_namespace = false);
		static byte* address_of_var(uint_t var_index, uint_t stack_level = 0, bool dont_dereference = false,
		                            bool return_address_of_unitialized_objects = false);
		static bool is_var_in_scope(uint_t var_index, uint_t stack_level = 0);
		static int_t this_type_id(uint_t stack_level = 0);
		static byte* this_pointer(uint_t stack_level = 0);
		static ScriptFunction system_function();
		friend class ScriptEngine;
	};
}// namespace Engine
