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
        CallbacksMap m_callbacks;
        Identifier m_id = 0;

    public:
        Identifier push(const Function<Signature>& callback)
        {
            m_callbacks[m_id] = callback;
            return m_id++;
        }

        Identifier push(Function<Signature>&& callback)
        {
            m_callbacks[m_id] = std::move(callback);
            return m_id++;
        }

        CallBacks& remove(Identifier ID)
        {
            m_callbacks.erase(ID);
            return *this;
        }

        template<typename... Args>
        const CallBacks& trigger(Args&&... args) const
        {
            for (auto& ell : m_callbacks)
            {
                ell.second(std::forward<Args>(args)...);
            }
            return *this;
        }

        const CallbacksMap& callbacks() const
        {
            return m_callbacks;
        }

        bool empty() const
        {
            return m_callbacks.empty();
        }

        template<typename... Args>
        const CallBacks& operator()(Args&&... args) const
        {
            return trigger(std::forward<Args>(args)...);
        }
    };
}// namespace Engine
