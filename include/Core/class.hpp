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
        Object* (*m_static_placement_constructor)(void*);
        Object* (*m_cast_to_this)(Object* object);
        Set<Class*> m_childs;
        size_t m_size;

        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

        template<typename T>
        static Object* constructor()
        {
            if constexpr (std::is_abstract_v<T> || (!std::is_default_constructible_v<T> && !Engine::is_singletone_v<T>) )
            {
                return nullptr;
            }
            else
            {
                return Object::new_instance<T>();
            }
        }

        template<typename T>
        static Object* placement_constructor(void* address)
        {
            if constexpr (std::is_abstract_v<T> || (!std::is_default_constructible_v<T> && !Engine::is_singletone_v<T>) )
            {
                return nullptr;
            }
            else
            {
                return Object::new_placement_instance<T>(address);
            }
        }

        void on_create_call(Object* object) const;
        void bind_class_to_script_engine();
        void register_scriptable_class();

    public:
        ScriptTypeInfo script_type_info;
        CallBacks<void(Object*)> on_create;
        CallBacks<void(Object*)> on_destroy;
        Function<void(ScriptClassRegistrar*, Class*)> script_registration_callback;

        Class(const Name& full_name, Class* parent = nullptr, BitMask flags = 0);

        Class* parent() const;
        const Set<Class*>& childs_classes() const;
        void* create_struct() const override;
        Object* create_object() const;
        Object* create_object(void* place) const;
        size_t sizeof_class() const;
        bool is_scriptable() const;
        Object* (*cast_to_this() const)(Object*);
        Object* (*static_constructor() const)();
        Object* (*static_placement_constructor() const)(void*);
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
                m_size                         = sizeof(ObjectClass);
                m_static_constructor           = constructor<ObjectClass>;
                m_static_placement_constructor = placement_constructor<ObjectClass>;

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
