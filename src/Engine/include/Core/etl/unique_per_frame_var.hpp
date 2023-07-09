#pragma once
#include <Event/event.hpp>
#include <functional>

namespace Engine
{
    template<typename Type>
    class UniquePerFrameVariable
    {
    private:
        Vector<Type> _M_vars;


        FORCE_INLINE UniquePerFrameVariable& check_count()
        {
            if (_M_vars.empty())
            {
                throw EngineException("Count of variables must be greater than 0");
            }
            return *this;
        }

    public:
        template<typename... Args>
        UniquePerFrameVariable& push(Args&&... args)
        {
            (_M_vars.push_back(std::forward<Args>(args)), ...);
            return *this;
        }

        template<typename Function, typename... Args>
        UniquePerFrameVariable& push_by_func(size_t count, Function&& function, Args&&... args)
        {
            for (size_t i = 0; i < count; i++)
                _M_vars.push_back(std::invoke(std::forward<Function>(function), std::forward<Args>(args)...));
            return *this;
        }

        FORCE_INLINE bool has_vars() const
        {
            return !_M_vars.empty();
        }

        FORCE_INLINE operator Type&()
        {
            return get();
        }

        FORCE_INLINE operator const Type&() const
        {
            return get();
        }

        FORCE_INLINE Index index() const
        {
            return 0;
            //return Event::frame_number() % _M_vars.size();
        }

        FORCE_INLINE Type& get()
        {
            return check_count()._M_vars[index()];
        }

        FORCE_INLINE const Type& get() const
        {
            return check_count()._M_vars[index()];
        }

        FORCE_INLINE Vector<Type>& variables()
        {
            return _M_vars;
        }

        FORCE_INLINE const Vector<Type>& variables() const
        {
            return _M_vars;
        }
    };
}// namespace Engine
