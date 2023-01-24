#pragma once
#include <set>

namespace Engine
{
    template<typename Return, typename... Args>
    class Callback final
    {
    public:
        using CallbackFunctionPrototype = Return (*)(Args...);

    private:
        std::set<CallbackFunctionPrototype> _M_callbacks;

    public:
        Callback& push(const CallbackFunctionPrototype& callback)
        {
            _M_callbacks.insert(callback);
            return *this;
        }

        Callback& remove(const CallbackFunctionPrototype& callback)
        {
            _M_callbacks.erase(callback);
            return *this;
        }

        const Callback& trigger(Args... args) const
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
