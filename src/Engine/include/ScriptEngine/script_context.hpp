#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/exception.hpp>

class asIScriptContext;

namespace Engine
{
    class ScriptFunction;
    class ScriptObject;

    class ENGINE_EXPORT ScriptContext
    {
    private:
        static void initialize();
        static void terminate();

        static int_t begin_execute(const ScriptFunction& function);
        static void end_execute(int_t begin_value, bool need_execute, void (*return_callback)(void* from, void* to) = nullptr,
                                void* copy_to = nullptr);

        template<typename T>
        static void copy_value(void* from, void* to)
        {
            if (from)
            {
                new (to) T(*reinterpret_cast<T*>(from));
            }
            else
            {
                throw EngineException("Failed to get address of return value!");
            }
        }

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

        static inline const ScriptObject& validate_argument(const ScriptObject& object)
        {
            return object;
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
        static bool arg(uint_t arg, const ScriptObject& object);
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
        template<typename Return = void, typename... Args>
        static inline Return execute(const ScriptFunction& function, const Args&... args)
        {
            const int_t result         = begin_execute(function);
            uint_t argument            = 0;
            bool bind_arguments_status = (arg(argument++, validate_argument(args)) && ...);

            if (!bind_arguments_status)
            {
                end_execute(result, false);
                throw EngineException("Failed to bind arguments to script function!");
            }

            if constexpr (std::is_same_v<void, Return>)
            {
                end_execute(result, true);
            }
            else
            {
                byte data[sizeof(Return)];
                end_execute(result, true, copy_value<Return>, data);
                return *reinterpret_cast<Return*>(data);
            }
        }

        // Exception handling
        static bool exception(const char* info, bool allow_catch = true);
        static int_t exception_line_number(int_t* column = 0, const char** section_name = 0);
        static ScriptFunction exception_function();
        static String exception_string();
        static bool will_exception_be_caught();
        // static bool exception_callback(asSFuncPtr callback, void* obj, int callConv);
        // static void clear_exception_callback();

        //         // Debugging
        //         virtual int SetLineCallback(asSFuncPtr callback, void* obj, int callConv)                            = 0;
        //         virtual void ClearLineCallback()                                                                     = 0;
        //         virtual asUINT GetCallstackSize() const                                                              = 0;
        static ScriptFunction function(uint_t stack_level = 0);
        //         virtual int GetLineNumber(asUINT stackLevel = 0, int* column = 0, const char** sectionName = 0)      = 0;
        //         virtual int GetVarCount(asUINT stackLevel = 0)                                                       = 0;
        //         virtual int GetVar(asUINT varIndex, asUINT stackLevel, const char** name, int* typeId = 0,
        //                            asETypeModifiers* typeModifiers = 0, bool* isVarOnHeap = 0, int* stackOffset = 0) = 0;
        //         virtual const char* GetVarDeclaration(asUINT varIndex, asUINT stackLevel = 0, bool includeNamespace = false) = 0;
        //         virtual void* GetAddressOfVar(asUINT varIndex, asUINT stackLevel = 0, bool dontDereference = false,
        //                                       bool returnAddressOfUnitializedObjects = false) = 0;
        //         virtual bool IsVarInScope(asUINT varIndex, asUINT stackLevel = 0)             = 0;
        //         virtual int GetThisTypeId(asUINT stackLevel = 0)                              = 0;
        //         virtual void* GetThisPointer(asUINT stackLevel = 0)                           = 0;
        static ScriptFunction system_function();
        friend class ScriptEngine;
    };
}// namespace Engine
