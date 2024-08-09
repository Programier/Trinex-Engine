#pragma once
#include <Core/callback.hpp>
#include <Core/enums.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>
#include <ScriptEngine/script_func_ptr.hpp>

namespace Engine
{
    class ENGINE_EXPORT Class final : public Struct
    {
    public:
        enum Flag : BitMask
        {
            IsSingletone = 1 << 0,
            IsAbstract   = 1 << 1,
            IsFinal      = 1 << 2,
            IsScriptable = 1 << 3,
            IsAsset      = 1 << 4,
        };

        struct MethodInfo {
            String declaration      = "";
            String name             = "";
            ScriptMethodPtr* method = nullptr;
            ScriptFuncPtr* function = nullptr;
            ScriptCallConv conv     = ScriptCallConv::CDecl;
            uint_t args_count       = 0;
            bool is_static          = false;
        };

        Flags<Class::Flag> flags;

    private:
        mutable Object* m_singletone_object;

        TreeMap<String, MethodInfo> m_methods;

        Object* (*m_static_constructor)();
        Object* (*m_cast_to_this)(Object* object);
        Set<Class*> m_childs;
        size_t m_size;

        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

        void on_create_call(Object* object) const;

    public:
        CallBacks<void(Object*)> on_create;
        CallBacks<void(Object*)> on_destroy;

        Class(const Name& full_name, Class* parent = nullptr, BitMask flags = 0);

        Class* parent() const;
        const Set<Class*>& childs_classes() const;
        void* create_struct() const override;
        Object* create_object() const;
        size_t sizeof_class() const;
        bool is_scriptable() const;
        Object* (*cast_to_this() const)(Object*);
        Object* (*static_constructor() const)();
        Object* singletone_instance() const;
        Class& post_initialize();

        using Struct::is_a;
        bool is_asset() const;
        bool is_class() const override;

        static Class* static_find(const StringView& name, bool required = false);
        static const Vector<Class*>& asset_classes();

        // Script class reflections
        const TreeMap<String, MethodInfo>& methods_info() const;
        ;
        Class& method(const char* declaration, ScriptMethodPtr* method, ScriptCallConv conv = ScriptCallConv::ThisCall);
        Class& method(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv = ScriptCallConv::CDeclObjFirst);

        template<typename ReturnType, typename ClassType, typename... Args>
        Class& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...),
                      ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return method(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename ClassType, typename... Args>
        Class& method(const char* declaration, ReturnType (ClassType::*method_address)(Args...) const,
                      ScriptCallConv conv = ScriptCallConv::ThisCall)
        {
            return method(declaration, ScriptMethodPtr::method_ptr(method_address), conv);
        }

        template<typename ReturnType, typename... Args>
        Class& method(const char* declaration, ReturnType (*function_address)(Args...),
                      ScriptCallConv conv = ScriptCallConv::CDeclObjFirst)
        {
            return method(declaration, ScriptFuncPtr::function_ptr(function_address), conv);
        }

        Class& static_function(const char* declaration, ScriptFuncPtr* function, ScriptCallConv conv = ScriptCallConv::CDecl);

        template<typename ReturnType, typename... Args>
        Class& static_function(const char* declaration, ReturnType (*function)(Args...),
                               ScriptCallConv conv = ScriptCallConv::CDecl)
        {
            return static_function(declaration, ScriptFuncPtr::function_ptr(function), conv);
        }

        ~Class();

        template<typename Type>
        bool is_a() const
        {
            return is_a(Type::static_class_instance());
        }

        template<typename ObjectClass>
        void setup_class()
        {
            if (m_size == 0)
            {
                m_size               = sizeof(ObjectClass);
                m_static_constructor = &ObjectClass::static_constructor;

                if constexpr (std::is_final_v<ObjectClass>)
                {
                    flags(Flag::IsFinal, true);
                }

                if constexpr (std::is_abstract_v<ObjectClass>)
                {
                    flags(Flag::IsAbstract, true);
                }

                if constexpr (is_singletone_v<ObjectClass>)
                {
                    flags(Flag::IsSingletone, true);
                }

                m_cast_to_this = private_cast_func<ObjectClass>;
            }
        }


        friend class ScriptClassRegistrar;
        friend class SingletoneBase;
    };

}// namespace Engine
