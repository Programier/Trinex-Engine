#pragma once
#include <Core/function.hpp>
#include <set>

namespace Engine
{

    template<typename ReturnType, typename... Args>
    using CallBack = Function<ReturnType, Args...>;

    template<typename Return, typename... Args>
    class CallBacks final
    {
    public:
        using CallbackFunctionPrototype = Return (*)(Args...);

    private:
        std::set<CallbackFunctionPrototype> _M_callbacks;

    public:
        CallBacks& push(const CallbackFunctionPrototype& callback)
        {
            _M_callbacks.insert(callback);
            return *this;
        }

        CallBacks& remove(const CallbackFunctionPrototype& callback)
        {
            _M_callbacks.erase(callback);
            return *this;
        }

        const CallBacks& trigger(Args... args) const
        {
            for (auto& ell : _M_callbacks) ell(args...);
            return *this;
        }

        const std::set<CallbackFunctionPrototype>& callbacks() const
        {
            return _M_callbacks;
        }
    };


}// namespace Engine
