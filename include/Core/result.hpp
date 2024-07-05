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
        std::optional<Type> m_result;
        String m_msg = "No result!";

    public:
        Result() = default;

        Result(const Type& value) : m_result(value)
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
            m_result.reset();
            m_msg = "No result";
            return *this;
        }

        Result& message(const String& msg)
        {
            m_msg = msg;
            return *this;
        }

        Result& result(const Type& value)
        {
            m_result = value;
            return *this;
        }

        const Type& result() const
        {
            if (m_result.has_value())
                return m_result.value();
            throw std::runtime_error(m_msg);
        }

        String message()
        {
            return m_msg;
        }

        bool has_value() const
        {
            return m_result.has_value();
        }
    };
}
