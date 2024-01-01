#pragma once
#include <Core/callback.hpp>
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/flags.hpp>
#include <Core/property.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    class ENGINE_EXPORT Class final
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
        String _M_name;
        String _M_namespace;
        String _M_base_name;

        Object* (*_M_static_constructor)();
        Object* (*_M_cast_to_this)(Object* object);

        Class* _M_parent;
        size_t _M_size;

        mutable Object* _M_singletone_object;
        Vector<Property*> _M_properties;

        /// SCRIPT PART
        CallBack<void(class ScriptClassRegistrar*, Class*)> _M_script_register_callback;


        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

    public:
        Class(const String& class_name, Object* (*) (), Class* parent = nullptr, BitMask flags = 0);

        Class* parent() const;
        const String& name() const;
        const String& namespace_name() const;
        const String& base_name() const;
        Object* create_object() const;
        bool contains_class(const Class* c) const;
        size_t sizeof_class() const;
        bool is_binded_to_script() const;
        Object* (*cast_to_this() const)(Object*);
        Object* (*static_constructor() const)();
        Object* singletone_instance() const;

        Class& set_script_registration_callback(const CallBack<void(class ScriptClassRegistrar*, Class*)>&);
        Class& post_initialize();
        bool is_asset() const;
        const Vector<Property*>& properties() const;
        ~Class();

        static Class* static_find_class(const String& name);
        static const Vector<Class*>& asset_classes();
        static const Map<String, Class*>& class_table();

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

        template<typename PropType>
        PropType* create_prop(const Name& name, const String& description, size_t offset, BitMask flags = 0)
        {
            PropType* prop = new PropType(name, description, offset, flags);
            _M_properties.push_back(prop);
            return prop;
        }


        friend class ScriptClassRegistrar;
    };
}// namespace Engine
