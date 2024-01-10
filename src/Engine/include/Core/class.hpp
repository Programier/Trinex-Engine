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

        Flags flags;

    private:
        mutable Object* _M_singletone_object;

        Object* (*_M_static_constructor)();
        Object* (*_M_cast_to_this)(Object* object);
        size_t _M_size;


        /// SCRIPT PART
        CallBack<void(class ScriptClassRegistrar*, Class*)> _M_script_register_callback;

        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

    public:
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

        static Class* static_find(const String& name, bool required = false);
        static const Vector<Class*>& asset_classes();

        using Struct::is_a;
        ~Class();

        template<typename Type>
        bool is_a() const
        {
            return contains_class(Type::static_class_instance());
        }


        template<typename ObjectClass>
        void process_type()
        {
            if (_M_size == 0)
            {
                _M_size = sizeof(ObjectClass);

                if constexpr (std::is_final_v<ObjectClass>)
                {
                    flags(static_cast<BitMask>(Flag::IsFinal), true);
                }

                if constexpr (std::is_abstract_v<ObjectClass>)
                {
                    flags(static_cast<BitMask>(Flag::IsAbstract), true);
                }

                if constexpr (is_singletone_v<ObjectClass>)
                {
                    flags(static_cast<BitMask>(Flag::IsSingletone), true);
                }

                _M_cast_to_this = private_cast_func<ObjectClass>;
            }
        }


        friend class ScriptClassRegistrar;
    };

}// namespace Engine
