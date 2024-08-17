#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/name.hpp>
#include <Core/object.hpp>
#include <Core/string_functions.hpp>
#include <Core/struct.hpp>
#include <ScriptEngine/script_type_info.hpp>

namespace Engine
{
    class ScriptClassRegistrar;

    class ENGINE_EXPORT Class final : public Struct
    {
    public:
        enum Flag : BitMask
        {
            IsSingletone = BIT(0),
            IsAbstract   = BIT(1),
            IsFinal      = BIT(2),
            IsNative     = BIT(3),
            IsScriptable = BIT(4),
            IsAsset      = BIT(5),
        };

        Flags<Class::Flag> flags;

    private:
        mutable Object* m_singletone_object;

        Object* (*m_static_constructor)(StringView, Object*);
        Object* (*m_static_placement_constructor)(void*, StringView, Object*);
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
        void bind_class_to_script_engine();
        void register_scriptable_class();

    public:
        ScriptTypeInfo script_type_info;
        CallBacks<void(Object*)> on_create;
        CallBacks<void(Object*)> on_destroy;
        CallBacks<void(Class*)> on_class_destroy;
        Function<void(ScriptClassRegistrar*, Class*)> script_registration_callback;

        Class(const Name& full_name, Class* parent = nullptr, BitMask flags = 0);

        Class* parent() const;
        const Set<Class*>& childs_classes() const;
        void* create_struct() const override;

        Object* create_object(StringView name = "", Object* owner = nullptr) const;
        Object* create_placement_object(void* place, StringView name = "", Object* owner = nullptr) const;

        size_t sizeof_class() const;
        bool is_scriptable() const;
        Object* (*cast_to_this() const)(Object*);
        Class& static_constructor(Object* (*new_static_constructor)(StringView, Object*) );
        Class& static_placement_constructor(Object* (*new_static_placement_constructor)(void*, StringView, Object*) );
        Object* singletone_instance() const;
        Class& post_initialize();

        using Struct::is_a;
        bool is_asset() const;
        bool is_class() const override;

        static Class* static_find(const StringView& name, bool required = false);
        static const Vector<Class*>& asset_classes();

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
                m_size = sizeof(ObjectClass);
                flags(Flag::IsNative, true);

                m_static_constructor = [](StringView name, Object* owner) -> Object* {
                    return Object::new_instance<ObjectClass, true>(name, owner);
                };

                m_static_placement_constructor = [](void* place, StringView name, Object* owner) -> Object* {
                    return Object::new_placement_instance<ObjectClass, true>(place, name, owner);
                };

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
