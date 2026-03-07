#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/ref.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/math/fwd.hpp>
#include <ScriptEngine/script_variable.hpp>
#include <angelscript.h>

class asIScriptContext;
class asIScriptFunction;

namespace Trinex
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
			i32 return_type_id = 0;
			bool is_active     = false;
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
		static u32 nest_count();

		static bool object(const ScriptObject& object);
		static bool object(const void* address);

		static bool arg_bool(u32 arg, bool value);
		static bool arg_byte(u32 arg, u8 value);
		static bool arg_word(u32 arg, u16 value);
		static bool arg_dword(u32 arg, u32 value);
		static bool arg_qword(u32 arg, u64 value);
		static bool arg_float(u32 arg, float value);
		static bool arg_double(u32 arg, double value);
		static bool arg_script_obj(u32 arg, const ScriptObject& obj);
		static bool arg_address(u32 arg, void* addr, bool is_object = false);
		static bool arg_var_type(u32 arg, void* ptr, i32 type_id);

		template<typename ValueType>
		static bool arg(u32 idx, ValueType&& value)
		{
			using T = std::decay_t<ValueType>;

			if constexpr (std::is_integral_v<T>)
			{
				if constexpr (std::is_same_v<T, bool>)
					return arg_bool(idx, value);
				else if constexpr (sizeof(T) == sizeof(u8))
					return arg_bool(idx, static_cast<u8>(value));
				else if constexpr (sizeof(T) == sizeof(u16))
					return arg_word(idx, static_cast<u16>(value));
				else if constexpr (sizeof(T) == sizeof(u32))
					return arg_dword(idx, static_cast<u32>(value));
				else if constexpr (sizeof(T) == sizeof(u64))
					return arg_qword(idx, static_cast<u64>(value));
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
		static bool arg(u32 idx, RRef<T>& ref)
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
		static bool arg(u32 idx, LRef<T> ref)
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

		static void* address_of_arg(u32 arg);

		// Return value
		static u8 return_byte();
		static u16 return_word();
		static u32 return_dword();
		static u64 return_qword();
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

			u32 argument = 0;
			(arg(argument++, args), ...);

			return end_execute(return_value);
		}

		template<typename... Args>
		static inline bool execute(const ScriptObject& self, const ScriptFunction& function, void* return_value = nullptr,
		                           const Args&... args)
		{
			begin_execute(function);

			u32 argument = 0;
			object(self), (arg(argument++, args), ...);

			return end_execute(return_value);
		}

		template<typename... Args>
		static inline bool execute(const void* self, const ScriptFunction& function, void* return_value = nullptr,
		                           const Args&... args)
		{
			begin_execute(function);

			u32 argument = 0;
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

		static u32 callstack_size();
		static ScriptFunction function(u32 stack_level = 0);
		static Vector2i line_position(u32 stack_level = 0, StringView* section_name = nullptr);
		static u32 var_count(u32 stack_level = 0);
		static bool var(u32 var_index, u32 stack_level, StringView* name, i32* type_id = 0,
		                ScriptTypeModifiers* modifiers = nullptr, bool* is_var_on_heap = 0, i32* stack_offset = 0);
		static String var_declaration(u32 var_index, u32 stack_level = 0, bool include_namespace = false);
		static u8* address_of_var(u32 var_index, u32 stack_level = 0, bool dont_dereference = false,
		                          bool return_address_of_unitialized_objects = false);
		static bool is_var_in_scope(u32 var_index, u32 stack_level = 0);
		static i32 this_type_id(u32 stack_level = 0);
		static u8* this_pointer(u32 stack_level = 0);
		static ScriptFunction system_function();
		friend class ScriptEngine;
	};
}// namespace Trinex
