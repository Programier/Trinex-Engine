#pragma once
#include "angelscript.h"
#include <Core/engine_types.hpp>
#include <Core/etl/ref.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>
#include <Core/flags.hpp>
#include <ScriptEngine/script_enums.hpp>
#include <ScriptEngine/script_variable.hpp>

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

        template<typename T>
        static inline byte validate_argument(const T& value)
            requires Concepts::is_byte<T>
        {
            return static_cast<byte>(value);
        }

        template<typename T>
        static inline word validate_argument(const T& value)
            requires Concepts::is_word<T>
        {
            return static_cast<word>(value);
        }

        template<typename T>
        static inline dword validate_argument(const T& value)
            requires Concepts::is_dword<T>
        {
            return static_cast<dword>(value);
        }

        template<typename T>
        static inline qword validate_argument(const T& value)
            requires Concepts::is_qword<T>
        {
            return static_cast<qword>(value);
        }

        template<typename T>
        static inline float validate_argument(const T& value)
            requires Concepts::is_float<T>
        {
            return static_cast<float>(value);
        }

        template<typename T>
        static inline double validate_argument(const T& value)
            requires Concepts::is_double<T>
        {
            return static_cast<double>(value);
        }

        template<typename T>
        static inline const RRef<T>& validate_argument(const RRef<T>& value)
        {
            return value;
        }

        template<typename T>
        static inline const LRef<T>& validate_argument(const LRef<T>& value)
        {
            return value;
        }

        template<typename T>
        static inline void* validate_argument(T* value)
        {
            std::decay_t<T>* non_const = const_cast<std::decay_t<T>*>(value);
            return non_const;
        }

        template<typename T>
        static inline void* validate_argument(const T& value)
            requires(!Concepts::is_byte<T> && !Concepts::is_word<T> && !Concepts::is_dword<T> && !Concepts::is_qword<T> &&
                     !Concepts::is_float<T> && !Concepts::is_double<T>)
        {
            return validate_argument(&value);
        }

        template<typename T>
        static bool arg(uint_t index, const RRef<T>& value)
        {
            return arg_reference(index, const_cast<T*>(value.address()));
        }

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
        static ScriptVariable end_execute(bool need_execute);

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

        // Arguments

        static bool arg(uint_t arg, byte value);
        static bool arg(uint_t arg, word value);
        static bool arg(uint_t arg, dword value);
        static bool arg(uint_t arg, qword value);
        static bool arg(uint_t arg, float value);
        static bool arg(uint_t arg, double value);
        static bool arg(uint_t arg, void* addr);
        static bool arg_reference(uint_t arg, void* addr);

        template<typename T>
        static bool arg(uint_t arg, RRef<T>& ref)
        {
            if constexpr (std::is_pointer_v<T>)
            {
                return arg_reference(arg, ref.get());
            }
            else
            {
                return arg_reference(arg, ref.address());
            }
        }

        template<typename T>
        static bool arg(uint_t arg, LRef<T> ref)
        {
            if constexpr (std::is_pointer_v<T>)
            {
                return arg_reference(arg, ref.get());
            }
            else
            {
                return arg_reference(arg, ref.address());
            }
        }

        static bool arg(uint_t arg, void* ptr, int_t type_id);
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
        static void* address_of_return_value();

        // Function execution
        template<typename... Args>
        static inline ScriptVariable execute(const ScriptFunction& function, const Args&... args)
        {
            if (!begin_execute(function))
                return {};
            uint_t argument            = 0;
            bool bind_arguments_status = (arg(argument++, validate_argument(args)) && ...);

            if (!bind_arguments_status)
            {
                end_execute(false);
                throw EngineException("Failed to bind arguments to script function!");
            }

            return end_execute(true);
        }

        template<typename... Args>
        static inline ScriptVariable execute(const ScriptObject& self, const ScriptFunction& function, const Args&... args)
        {
            if (!begin_execute(function))
                return {};
            object(self);

            uint_t argument            = 0;
            bool bind_arguments_status = (arg(argument++, validate_argument(args)) && ...);

            if (!bind_arguments_status)
            {
                end_execute(false);
                throw EngineException("Failed to bind arguments to script function!");
            }

            return end_execute(true);
        }

        // Exception handling
        static bool exception(const char* info, bool allow_catch = true);
        static bool exception(const String& info, bool allow_catch = true);
        static IntVector2D exception_line_position(StringView* section_name = nullptr);
        static ScriptFunction exception_function();
        static String exception_string();
        static bool will_exception_be_caught();

        // Debugging
        static bool line_callback(const Function<void(void*)>& function, void* userdata = nullptr);
        static bool line_callback(const ScriptFunction& function);
        static ScriptContext& clear_line_callback();

        static uint_t callstack_size();
        static ScriptFunction function(uint_t stack_level = 0);
        static IntVector2D line_position(uint_t stack_level = 0, StringView* section_name = nullptr);
        static uint_t var_count(uint_t stack_level = 0);
        static bool var(uint_t var_index, uint_t stack_level, StringView* name, int_t* type_id = 0,
                        Flags<ScriptTypeModifiers>* modifiers = nullptr, bool* is_var_on_heap = 0, int_t* stack_offset = 0);
        static String var_declaration(uint_t var_index, uint_t stack_level = 0, bool include_namespace = false);
        static void* address_of_var(uint_t var_index, uint_t stack_level = 0, bool dont_dereference = false,
                                    bool return_address_of_unitialized_objects = false);
        static bool is_var_in_scope(uint_t var_index, uint_t stack_level = 0);
        static int_t this_type_id(uint_t stack_level = 0);
        static void* this_pointer(uint_t stack_level = 0);
        static ScriptFunction system_function();
        friend class ScriptEngine;
    };
}// namespace Engine
