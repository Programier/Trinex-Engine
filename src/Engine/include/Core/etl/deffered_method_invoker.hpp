#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct DefferedMethodInvokerBase {
        virtual void invoke(void* object) = 0;
        virtual ~DefferedMethodInvokerBase(){};
    };

    template<typename ReturnType, typename Instance, typename... Args>
    class DefferedMethodInvoker : public DefferedMethodInvokerBase
    {
    private:
        ReturnType (Instance::*_M_method)(Args...);
        std::tuple<std::remove_cvref_t<Args>...> _M_args;

    public:
        DefferedMethodInvoker(ReturnType (Instance::*method)(Args...), const std::remove_cvref_t<Args>&... args)
            : _M_method(method), _M_args(std::make_tuple(args...))
        {}

        virtual void invoke(void* object) override
        {
            Instance* instance = reinterpret_cast<Instance*>(object);
            std::apply(_M_method, std::tuple_cat(std::make_tuple(instance), _M_args));
        }
    };
}// namespace Engine
