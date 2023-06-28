#pragma once

#include <Core/etl/type_traits.hpp>
#include <Core/predef.hpp>


namespace Engine
{
    template<typename Type>
    class Singletone
    {
    private:
        struct SingletoneDestructor {
            ~SingletoneDestructor()
            {
                Type::destroy();
            }
        };

    public:
        template<typename... Args>
        static Type* init_instance(Args&&... args)
        {
            if (!Type::_M_instance)
            {
                static SingletoneDestructor _M_destructor;

                if constexpr (is_object_based_v<Type>)
                {
#if TRINEX_OBJECT_HEADER_INCLUDED
                    Type::_M_instance = Object::new_instance<Type>(std::forward<Args>(args)...);
#else
                    static_assert(!is_object_based_v<Type>,
                                  "Please, add #include <Core/object.hpp> before #include <Core/etl/singletone.hpp>");
#endif
                }
                else
                {
                    Type::_M_instance = new Type(std::forward<Args>(args)...);
                }
            }

            return Type::_M_instance;
        }

        static Type* instance()
        {
            return Type::_M_instance;
        }

        static void destroy()
        {
            if (Type::_M_instance)
            {
                if constexpr (is_object_based_v<Type>)
                {
#if TRINEX_OBJECT_HEADER_INCLUDED
                    Object::begin_destroy(Type::_M_instance);
#else
                    static_assert(!is_object_based_v<Type>,
                                  "Please, add #include <Core/object.hpp> before #include <Core/etl/singletone.hpp>");
#endif
                }
                else
                {
                    delete Type::_M_instance;
                }
                Type::_M_instance = nullptr;
            }
        }

        friend struct SingletoneDestructor;
    };
}// namespace Engine
