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
        Identifier _M_id = 0;

    public:
        Identifier push(const Function<Signature>& callback)
        {
            _M_callbacks[_M_id] = callback;
            return _M_id++;
        }

        Identifier push(Function<Signature>&& callback)
        {
            _M_callbacks[_M_id] = std::move(callback);
            return _M_id++;
        }

        CallBacks& remove(Identifier ID)
        {
            _M_callbacks.erase(ID);
            return *this;
        }

        template<typename... Args>
        const CallBacks& trigger(Args&&... args) const
        {
            for (auto& ell : _M_callbacks)
            {
                ell.second(std::forward<Args>(args)...);
            }
            return *this;
        }

        const CallbacksMap& callbacks() const
        {
            return _M_callbacks;
        }
    };
}// namespace Engine
