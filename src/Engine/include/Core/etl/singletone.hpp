#pragma once

#include <Core/etl/type_traits.hpp>
#include <Core/object.hpp>
#include <Core/definitions.hpp>


namespace Engine
{

    class ENGINE_EXPORT SingletoneBase
    {
    protected:
        static void register_singletone_object(Object* object, const Class* _class);
    };

    template<typename Type, typename Parent = Object>
    class Singletone : public Parent, public SingletoneBase
    {
    public:
        static constexpr bool singletone = true;

        template<typename... Args>
        static Type* create_instance(Args&&... args)
        {
            if (!instance())
            {
                Type::_M_instance = Object::new_instance<Type>(std::forward<Args>(args)...);

                if constexpr (std::is_base_of_v<Object, Type>)
                {
                    register_singletone_object(Type::_M_instance, Type::static_class_instance());
                }
            }

            return instance();
        }

        FORCE_INLINE static Type* instance()
        {
            return Type::_M_instance;
        }

        Singletone& begin_destroy()
        {
            Object::begin_destroy(Type::_M_instance);
            return *this;
        }
    };
}// namespace Engine
