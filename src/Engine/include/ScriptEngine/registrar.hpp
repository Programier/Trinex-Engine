#pragma once
#include <Core/engine_types.hpp>
#include <ScriptEngine/script_enums.hpp>

class asIScriptEngine;

namespace Engine
{
    class Class;

    class ENGINE_EXPORT ScriptClassRegistrar
    {
    public:
        enum ClassFlags
        {
            Ref                      = (1 << 0),
            Value                    = (1 << 1),
            GC                       = (1 << 2),
            POD                      = (1 << 3),
            NoHandle                 = (1 << 4),
            Scopen                   = (1 << 5),
            Template                 = (1 << 6),
            AsHandle                 = (1 << 7),
            AppClass                 = (1 << 8),
            AppClassConstructor      = (1 << 9),
            AppClassDestructor       = (1 << 10),
            AppClassAssignment       = (1 << 11),
            AppClassCopyConstructor  = (1 << 12),
            AppClassC                = (AppClass + AppClassConstructor),
            AppClassCD               = (AppClassC + AppClassDestructor),
            AppClassCA               = (AppClassC + AppClassAssignment),
            AppClassCK               = (AppClassC + AppClassCopyConstructor),
            AppClassCDA              = (AppClassCD + AppClassAssignment),
            AppClassCDK              = (AppClassCD + AppClassConstructor),
            AppClassCAK              = (AppClassCA + AppClassConstructor),
            AppClassCDAK             = (AppClassCDA + AppClassConstructor),
            AppClassD                = (AppClass + AppClassDestructor),
            AppClassDA               = (AppClassD + AppClassAssignment),
            AppClassDK               = (AppClassD + AppClassCopyConstructor),
            AppClassDAK              = (AppClassDA + AppClassConstructor),
            AppClassA                = (AppClass + AppClassAssignment),
            AppClassAK               = (AppClassA + AppClassCopyConstructor),
            AppClassK                = (AppClass + AppClassCopyConstructor),
            AppClassMoreConstructors = (1 << 31),
            AppPrimitive             = (1 << 13),
            AppFloat                 = (1 << 14),
            AppArray                 = (1 << 15),
            AppClassAllInts          = (1 << 16),
            AppClassAllFloats        = (1 << 17),
            NoCount                  = (1 << 18),
            AppClassAlign8           = (1 << 19),
            ImplicitHandle           = (1 << 20),
            MaskValidFlags           = 0x801FFFFF,
        };

        struct ClassInfo {
            BitMask flags;
            size_t size;
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

    private:
        String _M_class_base_name;
        String _M_class_namespace_name;
        String _M_class_name;
        String _M_current_namespace;
        ClassInfo _M_info;

        asIScriptEngine* _M_engine;

        static void declare_as_class(Class* _class, const ClassInfo& info);
        ScriptClassRegistrar& private_register_method(const char* declaration, void* method, ScriptCallConv conv);
        ScriptClassRegistrar& private_register_static_method(const char* declaration, void* method,
                                                             ScriptCallConv conv);

        ScriptClassRegistrar& private_register_behaviour(ScriptClassBehave behave, const char* declaration,
                                                         void* method, bool is_method, ScriptCallConv conv);
        ScriptClassRegistrar& private_register_operator(const char* declaration, void* method, bool is_method,
                                                        ScriptCallConv conv);

        ScriptClassRegistrar& property(const char* declaration, void* prop);

        ScriptClassRegistrar& prepare_namespace(bool static_member = false);
        ScriptClassRegistrar& release_namespace();
        ScriptClassRegistrar& declare_as_class();

    public:
        ScriptClassRegistrar(class Class* _class);
        ScriptClassRegistrar(const String& full_name, const ClassInfo& info = {});
        const String& namespace_name() const;
        const String& class_base_name() const;
        const String& class_name() const;


        // Method registration
        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_method(declaration, &method_address, conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_method(declaration, &method_address, conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& method(const char* declaration, ReturnType (*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::CDECL)
        {
            return private_register_static_method(declaration, reinterpret_cast<void*>(method_address), conv);
        }


        // Property registration

        template<typename T, typename C>
        ScriptClassRegistrar& property(const char* declaration, T C::*prop)
        {
            return property(declaration, reinterpret_cast<void*>(*reinterpret_cast<size_t*>(&prop)));
        }

        ScriptClassRegistrar& static_property(const char* declaration, void* prop);

        ScriptClassRegistrar& require_type(const String& name, const ClassInfo& info = {});

        template<typename... Types>
        ScriptClassRegistrar& require_types()
        {
            (declare_as_class(Types::static_class_instance()), ...);
            return *this;
        }

        // Behaviour registration

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behave, const char* declaration,
                                     ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_behaviour(behave, declaration, &method_address, true, conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behave, const char* declaration,
                                     ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_behaviour(behave, declaration, &method_address, true, conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& behave(ScriptClassBehave behave, const char* declaration,
                                     ReturnType (*method_address)(Args...), ScriptCallConv conv = ScriptCallConv::CDECL)
        {
            return private_register_behaviour(behave, declaration, reinterpret_cast<void*>(method_address), false,
                                              conv);
        }

        // Operator registration
        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (ClassType::*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_operator(declaration, &method_address, true, conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (ClassType::*method_address)(Args...) const,
                                     ScriptCallConv conv = ScriptCallConv::THISCALL)
        {
            return private_register_operator(declaration, &method_address, true, conv);
        }

        template<typename ReturnType, typename... Args>
        ScriptClassRegistrar& opfunc(const char* declaration, ReturnType (*method_address)(Args...),
                                     ScriptCallConv conv = ScriptCallConv::CDECL_OBJFIRST)
        {
            return private_register_operator(declaration, reinterpret_cast<void*>(method_address), false, conv);
        }


        template<typename T>
        static ClassInfo create_type_info(BitMask additional_flags = 0)
        {
            ClassInfo info;
#if defined(_MSC_VER) || defined(_LIBCPP_TYPE_TRAITS) || (__GNUC__ >= 5) ||                                            \
        (defined(__clang__) && !defined(CLANG_PRE_STANDARD))
            // MSVC, XCode/Clang, and gnuc 5+
            // C++11 compliant code
            constexpr bool has_constructor =
                    std::is_default_constructible<T>::value && !std::is_trivially_default_constructible<T>::value;
            constexpr bool has_destructor = std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value;
            constexpr bool has_assignment_operator =
                    std::is_copy_assignable<T>::value && !std::is_trivially_copy_assignable<T>::value;
            constexpr bool has_copy_constructor =
                    std::is_copy_constructible<T>::value && !std::is_trivially_copy_constructible<T>::value;
#elif (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))) ||                               \
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
            // All other compilers and versions are assumed to use non C++11 compliant code until proven otherwise
            // Not fully C++11 compliant. The has_trivial checks were used while the standard was still
            // being elaborated, but were then removed in favor of the above is_trivially checks
            // http://stackoverflow.com/questions/12702103/writing-code-that-works-when-has-trivial-destructor-is-defined-instead-of-is
            // https://github.com/mozart/mozart2/issues/51
            constexpr bool has_constructor =
                    std::is_default_constructible<T>::value && !std::has_trivial_default_constructor<T>::value;
            constexpr bool has_destructor = std::is_destructible<T>::value && !std::has_trivial_destructor<T>::value;
            constexpr bool has_assignment_operator =
                    std::is_copy_assignable<T>::value && !std::has_trivial_copy_assign<T>::value;
            constexpr bool has_copy_constructor =
                    std::is_copy_constructible<T>::value && !std::has_trivial_copy_constructor<T>::value;
#endif
            constexpr bool is_float = std::is_floating_point<T>::value;
            constexpr bool is_primitive =
                    std::is_integral<T>::value || std::is_pointer<T>::value || std::is_enum<T>::value;
            constexpr bool is_class = std::is_class<T>::value;
            constexpr bool is_array = std::is_array<T>::value;

            if constexpr (is_float)
                info.flags = ScriptClassRegistrar::AppFloat;
            else if (is_primitive)
                info.flags = ScriptClassRegistrar::AppPrimitive;
            else if (is_class)
            {
                BitMask flags = ScriptClassRegistrar::AppClass;
                if (has_constructor)
                    flags |= ScriptClassRegistrar::AppClassConstructor;
                if (has_destructor)
                    flags |= ScriptClassRegistrar::AppClassDestructor;
                if (has_assignment_operator)
                    flags |= ScriptClassRegistrar::AppClassAssignment;
                if (has_copy_constructor)
                    flags |= ScriptClassRegistrar::AppClassCopyConstructor;
                info.flags = flags;

            }
            else if (is_array)
                info.flags = ScriptClassRegistrar::AppArray;


            info.size = sizeof(T);
            info.flags |= additional_flags;
            return info;
        }
    };


    class ENGINE_EXPORT ScriptEnumRegistrar
    {
    private:
        String _M_enum_base_name;
        String _M_enum_namespace_name;
        String _M_current_namespace;

        asIScriptEngine* _M_engine;

    public:
        ScriptEnumRegistrar(const String& full_name);
        ScriptEnumRegistrar& prepare_namespace();
        ScriptEnumRegistrar& release_namespace();

        ScriptEnumRegistrar& set(const char* name, int_t value);
    };
}// namespace Engine