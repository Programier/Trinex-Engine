#pragma once

namespace Engine
{
    template<typename Out, typename... Args>
    class ReturnFunctionWrapper
    {
    private:
        Out (*_M_function)(Args...) = nullptr;

    public:
        ReturnFunctionWrapper(Out (*function)(Args...)) : _M_function(function)
        {}

        ReturnFunctionWrapper& operator()(const Args&... args)
        {
            _M_function(args...);
            return *this;
        }

        const ReturnFunctionWrapper& operator()(const Args&... args) const
        {
            _M_function(args...);
            return *this;
        }

        auto operator*() const
        {
            return _M_function;
        }
    };
}
