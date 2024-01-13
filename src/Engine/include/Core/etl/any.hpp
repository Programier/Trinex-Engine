#pragma once
#include <Core/export.hpp>
#include <stdexcept>
#include <type_traits>

namespace Engine
{
    class ENGINE_EXPORT Any final
    {
    private:
        class ENGINE_EXPORT bad_any_cast : public std::bad_cast
        {
        public:
            const char* what() const noexcept override;
        };

        union ENGINE_EXPORT Storage
        {
            using stack_storage_t = typename std::aligned_storage<2 * sizeof(void*), std::alignment_of<void*>::value>::type;
            void* dynamic;
            stack_storage_t stack;

            Storage();
        };

        template<typename T>
        static constexpr inline bool is_stack_type = std::integral_constant<
                bool, std::is_nothrow_move_constructible<T>::value && sizeof(T) <= sizeof(Storage::stack) &&
                              std::alignment_of<T>::value <= std::alignment_of<Storage::stack_storage_t>::value>::value;

        struct ENGINE_EXPORT Manager {
            void (*destroy)(Storage&) noexcept;
            void (*copy)(const Storage& src, Storage& dest);
            void (*move)(Storage& src, Storage& dest) noexcept;
            void (*swap)(Storage& lhs, Storage& rhs) noexcept;


            Manager();
            bool is_valid() const;
            void reset();
        };

        template<typename T>
        struct StackManager {
            static void destroy(Storage& storage) noexcept
            {
                std::destroy_at(reinterpret_cast<T*>(&storage.stack));
            }

            static void copy(const Storage& src, Storage& dest)
            {
                new (&dest.stack) T(reinterpret_cast<const T&>(src.stack));
            }

            static void move(Storage& src, Storage& dest) noexcept
            {
                new (&dest.stack) T(std::move(reinterpret_cast<T&>(src.stack)));
                destroy(src);
            }

            static void swap(Storage& lhs, Storage& rhs) noexcept
            {
                Storage tmp_storage;
                move(rhs, tmp_storage);
                move(lhs, rhs);
                move(tmp_storage, lhs);
            }
        };

        template<typename T>
        struct DynamicManager {
            static void destroy(Storage& storage) noexcept
            {
                delete reinterpret_cast<T*>(storage.dynamic);
            }

            static void copy(const Storage& src, Storage& dest)
            {
                dest.dynamic = new T(*reinterpret_cast<const T*>(src.dynamic));
            }

            static void move(Storage& src, Storage& dest) noexcept
            {
                dest.dynamic = src.dynamic;
                src.dynamic  = nullptr;
            }

            static void swap(Storage& lhs, Storage& rhs) noexcept
            {
                std::swap(lhs.dynamic, rhs.dynamic);
            }
        };

        template<typename Type>
        static Manager* find_manager_of()
        {
            static Manager manager;
            if (!manager.is_valid())
            {
                if constexpr (is_stack_type<Type>)
                {
                    manager.destroy = StackManager<Type>::destroy;
                    manager.copy    = StackManager<Type>::copy;
                    manager.move    = StackManager<Type>::move;
                    manager.swap    = StackManager<Type>::swap;
                }
                else
                {
                    manager.destroy = DynamicManager<Type>::destroy;
                    manager.copy    = DynamicManager<Type>::copy;
                    manager.move    = DynamicManager<Type>::move;
                    manager.swap    = DynamicManager<Type>::swap;
                }
            }

            return &manager;
        }


        Storage _M_storage;
        Manager* _M_manager = nullptr;


        template<typename Value, typename DecayValue>
        typename std::enable_if<is_stack_type<DecayValue>>::type initialize(Value&& value)
        {
            new (&_M_storage.stack) DecayValue(std::forward<Value>(value));
        }


        template<typename Value, typename DecayValue>
        typename std::enable_if<!is_stack_type<DecayValue>>::type initialize(Value&& value)
        {
            _M_storage.dynamic = new DecayValue(std::forward<Value>(value));
        }

        template<typename Value>
        void construct(Value&& value)
        {
            using DecayValue = typename std::decay<Value>::type;
            _M_manager       = find_manager_of<DecayValue>();
            initialize<Value, DecayValue>(std::forward<Value>(value));
        }

        static void throw_exception() noexcept(false);

    public:
        Any();
        Any(const Any& any);
        Any(Any&& any);

        Any& operator=(const Any& any);
        Any& operator=(Any&& any);

        template<typename ValueType,
                 typename = typename std::enable_if<!std::is_same<typename std::decay<ValueType>::type, Any>::value>::type>
        Any(ValueType&& value)
        {
            static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
                          "Type must be copy constructible");
            construct(std::forward<ValueType>(value));
        }


        template<typename ValueType,
                 typename = typename std::enable_if<!std::is_same<typename std::decay<ValueType>::type, Any>::value>::type>
        Any& operator=(ValueType&& value)
        {
            static_assert(std::is_copy_constructible<typename std::decay<ValueType>::type>::value,
                          "Type must be copy constructible");
            Any(std::forward<ValueType>(value)).swap(*this);
            return *this;
        }

        bool has_value() const;
        Any& swap(Any& other);
        Any& reset();


        template<typename T>
        T cast()
        {
            using DecayT = std::decay_t<T>;
            if (has_value())
            {
                return is_stack_type<DecayT> ? *reinterpret_cast<DecayT*>(&_M_storage.stack)
                                             : *reinterpret_cast<DecayT*>(_M_storage.dynamic);
            }
            throw bad_any_cast();
        }

        template<typename T>
        const T cast() const
        {
            using DecayT = std::decay_t<T>;
            if (has_value())
            {
                return is_stack_type<DecayT> ? *reinterpret_cast<const DecayT*>(&_M_storage.stack)
                                             : *reinterpret_cast<const DecayT*>(_M_storage.dynamic);
            }
            throw bad_any_cast();
        }

        ~Any();
    };
}// namespace Engine
