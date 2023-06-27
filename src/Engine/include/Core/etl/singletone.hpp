#pragma once

#include <Core/etl/type_traits.hpp>
#include <Core/predef.hpp>


namespace Engine
{
    template<typename Type, bool thread_local_enable = false>
    struct ThreadLocalEnabler {
        static Type value;
    };

    template<typename Type>
    struct ThreadLocalEnabler<Type, false> {
        static Type value;
    };

    template<typename Type>
    struct ThreadLocalEnabler<Type, true> {
        thread_local static Type value;
    };

    template<typename Type>
    Type ThreadLocalEnabler<Type, false>::value;

    template<typename Type>
    thread_local Type ThreadLocalEnabler<Type, true>::value;


    template<typename Type, bool thread_local_enable = false>
    class ENGINE_EXPORT Singletone
    {
    private:
        class SingletoneDestructor
        {
        private:
            Type* _M_instance = nullptr;

        public:
            void operator=(Type* instance)
            {
                destruct();
                _M_instance = instance;
            }

            void destruct()
            {
                if (_M_instance)
                {
                    if constexpr (is_object_based_v<Type>)
                    {
#if TRINEX_OBJECT_HEADER_INCLUDED
                        Object::begin_destroy(_M_instance);
#else
                        static_assert(
                                !is_object_based_v<Type>,
                                "Please, add #include <Core/object.hpp> before #include <Core/etl/singletone.hpp>");
#endif
                    }
                    else
                    {
                        delete _M_instance;
                    }
                    _M_instance = nullptr;
                }
            }

            FORCE_INLINE operator bool()
            {
                return _M_instance != nullptr;
            }

            FORCE_INLINE Type* instance() const
            {
                return _M_instance;
            }

            ~SingletoneDestructor()
            {
                destruct();
            }
        };


        using WrapperType = ThreadLocalEnabler<SingletoneDestructor, thread_local_enable>;

        static WrapperType _M_destructor;

    public:
        template<typename... Args>
        static ENGINE_EXPORT Type* init_instance(Args&&... args)
        {
            if (!_M_destructor.value)
            {
                if constexpr (is_object_based_v<Type>)
                {
#if TRINEX_OBJECT_HEADER_INCLUDED
                    _M_destructor.value = Object::new_instance<Type>(std::forward<Args>(args)...);
#else
                    static_assert(!is_object_based_v<Type>,
                                  "Please, add #include <Core/object.hpp> before #include <Core/etl/singletone.hpp>");
#endif
                }
                else
                {
                    _M_destructor.value = new Type(std::forward<Args>(args)...);
                }
            }

            return _M_destructor.value.instance();
        }

        static ENGINE_EXPORT Type* instance()
        {
            return _M_destructor.value.instance();
        }

        static ENGINE_EXPORT void destroy()
        {
            _M_destructor.value.destruct();
        }

        friend class SingletoneDestructor;
    };

    template<typename Type, bool thread_local_enable>
    Singletone<Type, thread_local_enable>::WrapperType Singletone<Type, thread_local_enable>::_M_destructor;

}// namespace Engine
