#pragma once

#include <Core/definitions.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/type_traits.hpp>
#include <Core/object.hpp>


namespace Engine
{
    class ENGINE_EXPORT SingletoneBase
    {
    protected:
        static void allocate_instance(const Class* class_instance);
        static void begin_destroy_instance(const Class* class_instance);
        static Object* extract_object_from_class(const Class* class_instance);
    };

    template<typename Type, typename Parent = Object>
    class Singletone : public Parent, private SingletoneBase
    {
    public:
        static constexpr inline bool singletone_based_on_object = std::is_base_of_v<Object, Type>;

        template<typename... Args>
        static Type* create_instance(Args&&... args)
        {
            if (!instance())
            {
                if constexpr (singletone_based_on_object)
                {
                    allocate_instance(Type::static_class_instance());
                }
                else
                {
                    Type::_M_instance = Object::new_instance<Type>(std::forward<Args>(args)...);
                }
            }

            return instance();
        }

        FORCE_INLINE static Type* instance()
        {
            if constexpr (singletone_based_on_object)
            {
                Object* object = extract_object_from_class(Type::static_class_instance());
                if (object)
                {
                    return object->instance_cast<Type>();
                }

                return nullptr;
            }
            else
            {
                return Type::_M_instance;
            }
        }


        ~Singletone()
        {
            if constexpr (!singletone_based_on_object)
            {
                Type::_M_instance = nullptr;
            }
        }
    };
}// namespace Engine
