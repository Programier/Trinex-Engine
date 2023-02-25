#pragma once
#include <Core/engine_types.hpp>
#include <optional>
#include <stdexcept>

namespace Engine
{
    template<typename Type>
    class Result
    {
    private:
        std::optional<Type> _M_result;
        String _M_msg = "No result!";

    public:
        Result() = default;

        Result(const Type& value) : _M_result(value)
        {}

        Result(const Result&) = default;
        Result(Result&&) = default;
        Result& operator=(const Result&) = default;
        Result& operator=(Result&&) = default;

        operator Type()
        {
            return result();
        }


        Result& reset()
        {
            _M_result.reset();
            _M_msg = "No result";
            return *this;
        }

        Result& message(const String& msg)
        {
            _M_msg = msg;
            return *this;
        }

        Result& result(const Type& value)
        {
            _M_result = value;
            return *this;
        }

        const Type& result() const
        {
            if (_M_result.has_value())
                return _M_result.value();
            throw std::runtime_error(_M_msg);
        }

        String message()
        {
            return _M_msg;
        }

        bool has_value() const
        {
            return _M_result.has_value();
        }
    };
}
