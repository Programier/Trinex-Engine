#pragma once
#include <Core/engine_types.hpp>
#include <Core/exception.hpp>
#include <cstring>


namespace Engine
{
    class ENGINE_EXPORT AnyBase
    {
    protected:
        using Destructor    = void (*)(byte*);
        using ConstructCopy = void (*)(const byte*, byte*);

        Buffer _M_buffer;
        Destructor _M_destructor;
        ConstructCopy _M_copy_constructor;

        const std::type_info* _M_info;

        AnyBase& resize(size_t new_size);

    public:
        AnyBase();
        AnyBase(const AnyBase&);
        AnyBase(AnyBase&&);
        AnyBase& operator=(AnyBase&&);
        AnyBase& operator=(const AnyBase&);

        AnyBase& clear();
        size_t size() const;
        byte* data();
        const byte* data() const;
        bool empty() const;

        ~AnyBase();
    };


    class Any : public AnyBase
    {
    private:
        template<typename T>
        static void static_destructor(byte* address)
        {
            std::destroy_at(reinterpret_cast<T*>(address));
        }

        template<typename T>
        static void static_copy_constructor(const byte* _from, byte* _to)
        {
            const T* from = reinterpret_cast<const T*>(_from);
            T* to         = reinterpret_cast<T*>(_to);

            if constexpr (std::is_copy_constructible_v<T>)
            {
                new (to) T(*from);
            }
            else if constexpr (std::is_copy_assignable_v<T>)
            {
                (*to) = (*from);
            }
            else
            {
                throw EngineException("Cannot copy non-copiable instance!");
            }
        }

        template<typename T>
        FORCE_INLINE void copy(T&& value)
        {
            using DecayT = std::decay_t<T>;
            resize(sizeof(DecayT));
            new (_M_buffer.data()) DecayT(std::forward<T>(value));
            _M_destructor       = static_destructor<DecayT>;
            _M_copy_constructor = static_copy_constructor<DecayT>;
            _M_info             = &typeid(DecayT);
        }

        template<typename T>
        FORCE_INLINE const Any& check_type() const
        {
            if (std::strcmp(typeid(std::decay_t<T>).name(), _M_info->name()) != 0)
            {
                throw EngineException("Bad any cast");
            }
            return *this;
        }

        template<typename T>
        FORCE_INLINE Any& check_type()
        {
            if (std::strcmp(typeid(std::decay_t<T>).name(), _M_info->name()) != 0)
            {
                throw EngineException("Bad any cast");
            }
            return *this;
        }

    public:
        using AnyBase::AnyBase;

        template<typename T>
        Any(T&& value)
        {
            copy(std::forward<T>(value));
        }

        template<typename T>
        FORCE_INLINE Any& operator=(T&& value)
        {
            copy(std::forward<T>(value));
            return *this;
        }


        template<typename T>
        FORCE_INLINE T get()
        {
            using DecayT = std::decay_t<T>;
            if(sizeof(DecayT) != _M_buffer.size())
            {
                throw EngineException("Bad any cast");
            }
            DecayT* ptr  = reinterpret_cast<DecayT*>(_M_buffer.data());
            return *ptr;
        }

        template<typename T>
        FORCE_INLINE T get() const
        {
            using DecayT      = std::decay_t<T>;
            if(sizeof(DecayT) != _M_buffer.size())
            {
                throw EngineException("Bad any cast");
            }
            const DecayT* ptr = reinterpret_cast<const DecayT*>(_M_buffer.data());
            return *ptr;
        }

        template<typename T>
        FORCE_INLINE T checked_get()
        {
            return check_type<T>().template get<T>();
        }

        template<typename T>
        FORCE_INLINE T checked_get() const
        {
            return check_type<T>().template get<T>();
        }
    };

}// namespace Engine
