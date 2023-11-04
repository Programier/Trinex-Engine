#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/string_functions.hpp>

namespace Engine
{
    class ENGINE_EXPORT Class
    {
    public:
        enum Flags : EnumerateType
        {
            IsSingletone = 0,
            IsAbstract,
            IsFinal,
            __COUNT__
        };

    private:
        String _M_name;
        String _M_namespace;
        String _M_base_name;
        BitSet<Flags::__COUNT__> _M_flags;

        Object* (*_M_static_constructor)();
        Object* (*_M_cast_to_this)(Object* object);

        Class* _M_parent;
        size_t _M_size;
        bool _M_is_script_registered;
        mutable Object* _M_singletone_object;

        static Object* internal_cast(Class* required_class, Object* object);

        template<typename T>
        static Object* private_cast_func(Object* o)
        {
            return internal_cast(T::static_class_instance(), o);
        }

    public:
        Class(const String& class_name, Object* (*) (), Class* parent = nullptr);

        Class* parent() const;
        const String& name() const;
        const String& namespace_name() const;
        const String& base_name() const;
        Object* create_object() const;
        static Class* static_find_class(const String& name);
        bool contains_class(const Class* c) const;
        size_t sizeof_class() const;
        bool is_binded_to_script() const;
        Object* (*cast_to_this() const)(Object*);
        Object* (*static_constructor() const)();

        Object* singletone_instance() const;

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
                _M_size                = sizeof(ObjectClass);
                _M_flags[IsFinal]      = std::is_final_v<ObjectClass>;
                _M_flags[IsAbstract]   = std::is_abstract_v<ObjectClass>;
                _M_flags[IsSingletone] = is_singletone_v<ObjectClass>;

                _M_cast_to_this = private_cast_func<ObjectClass>;
            }
        }

        bool has_any_flags(BitMask flags) const;
        bool has_all_flags(BitMask flags) const;

        friend class ScriptClassRegistrar;
    };
}// namespace Engine
