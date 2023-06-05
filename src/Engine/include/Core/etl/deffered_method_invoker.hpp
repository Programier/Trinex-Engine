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
        Tuple<Args...> _M_args;

        template<std::size_t... Indices>
        void private_invoke(Instance* instance, std::index_sequence<Indices...>&&)
        {
            ((*instance).*_M_method)(std::forward<Args>(std::get<Indices>(_M_args))...);
        }

    public:
        template<typename... MethodArgs>
        DefferedMethodInvoker(ReturnType (Instance::*method)(Args...), MethodArgs&&... args)
            : _M_method(method), _M_args(std::forward<MethodArgs>(args)...)
        {}

        virtual void invoke(void* object) override
        {
            Instance* instance = reinterpret_cast<Instance*>(object);
            private_invoke(instance, std::index_sequence_for<Args...>());
        }
    };
}// namespace Engine
