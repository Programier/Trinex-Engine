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
        using CallbacksMap = TreeMap<Identifier, CallBack<Signature>>;

    private:
        CallbacksMap _M_callbacks;

    public:
        Identifier push(const Function<Signature>& callback)
        {
            Identifier id    = _M_callbacks.empty() ? 0 : _M_callbacks.begin()->first + 1;
            _M_callbacks[id] = callback;
            return id;
        }

        Identifier push(Function<Signature>&& callback)
        {
            Identifier id    = _M_callbacks.empty() ? 0 : _M_callbacks.begin()->first + 1;
            _M_callbacks[id] = std::move(callback);
            return id;
        }

        CallBacks& remove(Identifier ID)
        {
            _M_callbacks.erase(ID);
            return *this;
        }

        template<typename... Args>
        const CallBacks& trigger(Args&&... args) const
        {
            for (auto& ell : _M_callbacks) ell.second(std::forward<Args>(args)...);
            return *this;
        }

        const CallbacksMap& callbacks() const
        {
            return _M_callbacks;
        }
    };
}// namespace Engine
