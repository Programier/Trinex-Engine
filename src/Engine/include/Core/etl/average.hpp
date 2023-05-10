#pragma once
#include <type_traits>

namespace Engine
{
    template<typename Type, typename = typename std::enable_if<std::is_arithmetic<Type>::value>::type>
    class Average
    {
        std::size_t _M_count = 0;
        Type _M_value        = static_cast<Type>(0);

    public:
        Average& push(const Type& value)
        {
            ++_M_count;

            if (_M_count > 1)
            {
                _M_value /= (static_cast<Type>(_M_count) / static_cast<Type>(_M_count - 1));
            }

            _M_value += value / static_cast<Type>(_M_count);
            return *this;
        }

        template<typename... Args>
        Average& push(const Type& value, Args... args)
        {
            push(value);
            for (auto& ell : {args...})
            {
                push(ell);
            }

            return *this;
        }

        operator Type() const
        {
            return _M_value;
        }

        const Type& average() const
        {
            return _M_value;
        }

        Average& reset()
        {
            _M_value = static_cast<Type>(0);
            _M_count = 0;
            return *this;
        }

        std::size_t count() const
        {
            return _M_count;
        }
    };
}
