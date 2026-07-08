#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>
#include <Core/etl/function.hpp>
#include <Core/etl/ref.hpp>
#include <Core/etl/string.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/math/fwd.hpp>
#include <ScriptEngine/enums.hpp>
#include <angelscript.h>

class asIScriptContext;
class asIScriptFunction;

namespace Trinex
{
	class ScriptFunction;
	class ScriptObject;

	class ENGINE_EXPORT ScriptContext
	{
		asIScriptContext* m_context           = nullptr;
		Function<void(void*)> m_line_callback = {};

		void initialize_callbacks();
		void release_context();

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

	public:
		static ScriptContext* current();
		static ScriptContext* local();

	public:
		ScriptContext();
		ScriptContext(const ScriptContext&) = delete;
		ScriptContext(ScriptContext&& other) noexcept;
		ScriptContext& operator=(const ScriptContext&) = delete;
		ScriptContext& operator=(ScriptContext&& other) noexcept;

		bool is_valid() const;
		asIScriptContext* context() const;
		void trigger_line_callback();

		bool begin_execute(const ScriptFunction& function);
		bool end_execute(void* return_value = nullptr);

		bool prepare(const ScriptFunction& func);
		bool unprepare();
		bool execute();
		bool abort();
		bool suspend();
		State state() const;
		bool push_state();
		bool pop_state();
		u32 nest_count() const;

		bool object(const void* address);
		bool arg_bool(u32 arg, bool value);
		bool arg_byte(u32 arg, u8 value);
		bool arg_word(u32 arg, u16 value);
		bool arg_dword(u32 arg, u32 value);
		bool arg_qword(u32 arg, u64 value);
		bool arg_float(u32 arg, float value);
		bool arg_double(u32 arg, double value);
		bool arg_script_obj(u32 arg, const void* object);
		bool arg_address(u32 arg, void* addr, bool is_object = false);
		bool arg_var_type(u32 arg, void* ptr, i32 type_id);

		template<typename ValueType>
		bool arg(u32 idx, ValueType&& value)
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
		bool arg(u32 idx, RRef<T>& ref)
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
		bool arg(u32 idx, LRef<T> ref)
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

		void* address_of_arg(u32 arg) const;

		u8 return_byte() const;
		u16 return_word() const;
		u32 return_dword() const;
		u64 return_qword() const;
		float return_float() const;
		double return_double() const;
		void* return_address() const;
		void* return_object_ptr() const;
		void* address_of_return_value() const;

		template<typename... Args>
		bool execute(const ScriptFunction& function, void* return_value = nullptr, const Args&... args)
		{
			if (!begin_execute(function))
				return false;

			u32 argument = 0;
			(arg(argument++, args), ...);

			return end_execute(return_value);
		}

		template<typename... Args>
		bool execute(const void* self, const ScriptFunction& function, void* return_value = nullptr, const Args&... args)
		{
			if (!begin_execute(function))
				return false;

			u32 argument = 0;
			object(self), (arg(argument++, args), ...);

			return end_execute(return_value);
		}

		bool exception(const char* info, bool allow_catch = true);
		bool exception(const String& info, bool allow_catch = true);
		Vector2i exception_line_position(StringView* section_name = nullptr) const;
		ScriptFunction exception_function() const;
		String exception_string() const;
		bool will_exception_be_caught() const;

		bool line_callback(const Function<void(void*)>& function, void* userdata = nullptr);
		bool line_callback(const ScriptFunction& function);
		ScriptContext& clear_line_callback();

		u32 callstack_size() const;
		ScriptFunction function(u32 stack_level = 0) const;
		Vector2i line_position(u32 stack_level = 0, StringView* section_name = nullptr) const;
		u32 var_count(u32 stack_level = 0) const;
		bool var(u32 var_index, u32 stack_level, StringView* name, i32* type_id = 0, ScriptTypeModifiers* modifiers = nullptr,
		         bool* is_var_on_heap = 0, i32* stack_offset = 0) const;
		String var_declaration(u32 var_index, u32 stack_level = 0, bool include_namespace = false) const;
		u8* address_of_var(u32 var_index, u32 stack_level = 0, bool dont_dereference = false,
		                   bool return_address_of_unitialized_objects = false) const;
		bool is_var_in_scope(u32 var_index, u32 stack_level = 0) const;
		i32 this_type_id(u32 stack_level = 0) const;
		u8* this_pointer(u32 stack_level = 0) const;
		ScriptFunction system_function() const;

		~ScriptContext();

		friend class ScriptEngine;
	};
}// namespace Trinex
