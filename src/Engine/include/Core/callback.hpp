#pragma once
#include <Core/engine_types.hpp>


namespace Engine
{

    template<typename Signature>
    using CallBack = Function<Signature>;

    template<typename Signature>
    class CallBacks final
    {
    public:
        using CallbacksArray = Vector<CallBack<Signature>>;

    private:
        CallbacksArray _M_callbacks;

    public:
        CallBacks& push(const Function<Signature>& callback)
        {
            _M_callbacks.push_back(callback);
            return *this;
        }

        CallBacks& push(Function<Signature>&& callback)
        {
            _M_callbacks.push_back(std::move(callback));
            return *this;
        }

        CallBacks& remove(const Function<Signature>& callback)
        {
            for (auto it = _M_callbacks.begin(); it != _M_callbacks.end(); ++it)
            {
                if(*it == callback)
                {
                    _M_callbacks.erase(it);
                }
            }
            return *this;
        }

        template<typename... Args>
        const CallBacks& trigger(Args&&... args) const
        {
            for (auto& ell : _M_callbacks) ell(std::forward<Args>(args)...);
            return *this;
        }

        const CallbacksArray& callbacks() const
        {
            return _M_callbacks;
        }
    };
}// namespace Engine
