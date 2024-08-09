#pragma once
#include <Core/enums.hpp>
#include <ScriptEngine/script_func_ptr.hpp>

class asIScriptEngine;

namespace Engine
{
    class Class;

    class ENGINE_EXPORT ScriptClassRegistrar
    {
    public:
        struct ENGINE_EXPORT BaseInfo {
            String template_type;
            BitMask extra_flags = 0;

            BaseInfo();
        };

        struct ENGINE_EXPORT ValueInfo : public BaseInfo {
            bool all_ints : 1;
            bool all_floats : 1;
            bool pod : 1;
            bool more_constructors : 1;
            bool is_class : 1;
            bool is_array : 1;
            bool is_float : 1;
            bool is_primitive : 1;
            bool has_constructor : 1;
            bool has_destructor : 1;
            bool has_assignment_operator : 1;
            bool has_copy_constructor : 1;

            ValueInfo();

            template<typename T>
            static ValueInfo from(ValueInfo info = ValueInfo())
            {
#if defined(_MSC_VER) || defined(_LIBCPP_TYPE_TRAITS) || (__GNUC__ >= 5) || (defined(__clang__) && !defined(CLANG_PRE_STANDARD))
                // MSVC, XCode/Clang, and gnuc 5+
                // C++11 compliant code
                constexpr bool has_constructor =
                        std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value;
                constexpr bool has_destructor = std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value;
                constexpr bool has_assignment_operator =
                        std::is_copy_assignable<T>::value && !std::is_trivially_copy_assignable<T>::value;
                constexpr bool has_copy_constructor =
                        std::is_copy_constructible<T>::value && !std::is_trivially_copy_constructible<T>::value;
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) ||                                         \
        (defined(__clang__) && defined(CLANG_PRE_STANDARD))
                // gnuc 4.8 is using a mix of C++11 standard and pre-standard templates
                constexpr bool has_constructor =
                        std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
                constexpr bool has_destructor = std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value;
                constexpr bool has_assignment_operator =
                        std::is_copy_assignable<T>::value && !std::has_trivial_copy_assign<T>::value;
                constexpr bool has_copy_constructor =
                        std::is_copy_constructible<T>::value && !std::has_trivial_copy_constructor<T>::value;
#else
                constexpr bool has_constructor =
                        std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
                constexpr bool has_destructor = std::is_destructible<T>::value && !std::has_trivial_destructor<T>::value;
                constexpr bool has_assignment_operator =
                        std::is_copy_assignable<T>::value && !std::has_trivial_copy_assign<T>::value;
                constexpr bool has_copy_constructor =
                        std::is_copy_constructible<T>::value && !std::has_trivial_copy_constructor<T>::value;
#endif
                info.is_class                = false;
                info.is_array                = false;
                info.is_primitive            = false;
                info.has_constructor         = false;
                info.has_destructor          = false;
                info.has_assignment_operator = false;

                if (std::is_floating_point<T>::value)
                {
                    info.is_float = true;
                }
                else if (std::is_integral<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value)
                {
                    info.is_primitive = true;
                }
                else if (std::is_class<T>::value)
                {
                    info.is_class                = true;
                    info.has_constructor         = has_constructor;
                    info.has_destructor          = has_destructor;
                    info.has_assignment_operator = has_assignment_operator;
                    info.has_copy_constructor    = has_copy_constructor;
                }
                else if (std::is_array<T>::value)
                {
                    info.is_array = true;
                }
                return info;
            }
        };

        struct ENGINE_EXPORT RefInfo : public BaseInfo {
            bool no_count : 1;
            bool implicit_handle : 1;

            RefInfo();
        };

        template<typename T, typename... Args>
        static void constructor(T* memory, Args... args)
        {
            byte* data = reinterpret_cast<byte*>(memory);
            std::fill(data, data + sizeof(T), 0);
            new (memory) T(args...);
        }

        template<typename T>
        static void destructor(T* memory)
        {
            memory->~T();
        }

        template<typename T, typename B>
        static T& assign(T* self, B other)
        {
            (*self) = other;
            return *self;
        }


    private:
        String m_class_name;
        String m_class_base_name;
        String m_namespace_name;


        ScriptClassRegistrar(const StringView& name);
        ScriptClassRegistrar(const StringView& name, size_t size, BitMask flags);

        ScriptClassRegistrar& modify_name_if_template(const BaseInfo& info);

    public:
        static ScriptClassRegistrar value_class(const StringView& name, size_t size, const ValueInfo& info = ValueInfo());
        static ScriptClassRegistrar reference_class(const StringView& name, const RefInfo& info = RefInfo());
        static ScriptClassRegistrar existing_class(const String& name);

        const String& class_name() const;
        const String& class_base_name() const;
        const String& namespace_name() const;

        // Method registration
        ScriptClassRegistrar& method(const char* declaration, ScriptMethodPtr* method,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall);
        ScriptClassRegistrar& method(const char* declaration, ScriptFuncPtr* function,
                                     ScriptCallConv conv = ScriptCallConv::CDeclObjFirst);

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return method(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return method(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (*function_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::CDeclObjFirst)
        {
            return method(declaration, ScriptFuncPtr::function_ptr(function_address), conv);
        }

        ScriptClassRegistrar& static_function(const char* declaration, ScriptFuncPtr* function,
                                              ScriptCallConv conv = ScriptCallConv::CDecl);

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& static_function(const char* declaration, ReturnType (*function)(Args...),
                                              ScriptCallConv conv = ScriptCallConv::CDecl)
        {
            return static_function(declaration, ScriptFuncPtr::function_ptr(function), conv);
        }

        // Property registration
        ScriptClassRegistrar& property(const char* declaration, size_t offset);

        template<typename T, typename C>
        ScriptClassRegistrar& property(const char* declaration, T C::*prop)
        {
            return property(declaration, offset_of(prop));
        }

        ScriptClassRegistrar& static_property(const char* declaration, void* prop);

        // Behaviour registration
        ScriptClassRegistrar& behave(ScriptClassBehave behaviour, const char* declaration, ScriptFuncPtr* function,
                                     ScriptCallConv conv = ScriptCallConv::CDeclObjFirst);
        ScriptClassRegistrar& behave(ScriptClassBehave behaviour, const char* declaration, ScriptMethodPtr* method,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall);

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behaviour, const char* declaration,
                                     ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return behave(behaviour, declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behaviour, const char* declaration,
                                     ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return behave(behaviour, declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behaviour, const char* declaration,
                                     ReturnType (*function_address)(Args...), ScriptCallConv conv = ScriptCallConv::CDeclObjFirst)
        {
            return behave(behaviour, declaration, ScriptFuncPtr::function_ptr(function_address), conv);
        }

        // Operator registration

        ScriptClassRegistrar& opfunc(const char* declaration, ScriptMethodPtr* method,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall);
        ScriptClassRegistrar& opfunc(const char* declaration, ScriptFuncPtr* function,
                                     ScriptCallConv conv = ScriptCallConv::CDeclObjFirst);

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return opfunc(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return opfunc(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (*function)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::CDeclObjFirst)
        {
            return opfunc(declaration, ScriptFuncPtr::function_ptr(function), conv);
        }
    };


    class ENGINE_EXPORT ScriptEnumRegistrar
    {
    private:
        String m_enum_base_name;
        String m_enum_namespace_name;
        String m_current_namespace;

        ScriptEnumRegistrar& prepare_namespace();
        ScriptEnumRegistrar& release_namespace();

    public:
        ScriptEnumRegistrar(const String& namespace_name, const String& base_name, bool init = true);
        ScriptEnumRegistrar(const String& full_name, bool init = true);

        ScriptEnumRegistrar& set(const char* name, int_t value);

        template<typename T>
        ScriptEnumRegistrar& set(const char* name, T value)
        {
            return set(name, static_cast<int_t>(value));
        }
    };
}// namespace Engine
