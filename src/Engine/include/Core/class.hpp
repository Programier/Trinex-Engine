#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>

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

        Flags<Class::Flag> flags;

    private:
        mutable Object* m_singletone_object;

        Object* (*m_static_constructor)();
        Object* (*m_cast_to_this)(Object* object);
        size_t m_size;


        /// SCRIPT PART
        CallBack<void(class ScriptClassRegistrar*, Class*)> m_script_register_callback;

        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

        void on_create_call(Object* object) const;

    public:
        CallBacks<void(Object*)> on_create;

        Class(const Name& name, const Name& namespace_name, Object* (*) (), Class* parent = nullptr, BitMask flags = 0);

        Class* parent() const;
        void* create_struct() const override;
        Object* create_object() const;
        size_t sizeof_class() const;
        bool is_binded_to_script() const;
        Object* (*cast_to_this() const)(Object*);
        Object* (*static_constructor() const)();
        Object* singletone_instance() const;

        Class& set_script_registration_callback(const CallBack<void(class ScriptClassRegistrar*, Class*)>&);
        Class& post_initialize();
        bool is_asset() const;
        bool is_class() const override;

        static Class* static_find(const StringView& name, bool required = false);
        static const Vector<Class*>& asset_classes();

        using Struct::is_a;
        ~Class();

        template<typename Type>
        bool is_a() const
        {
            return is_a(Type::static_class_instance());
        }


        template<typename ObjectClass>
        void process_type()
        {
            if (m_size == 0)
            {
                m_size = sizeof(ObjectClass);

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
