#pragma once

namespace Engine
{

    template<typename Return, typename... Args>
    constexpr Return (*func_of(Return (*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    constexpr Return (Instance::*func_of(Return (Instance::*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    constexpr Return (Instance::*func_of(Return (Instance::*function)(Args...) const))(Args...) const
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    constexpr Return (Instance::*method_of(Return (Instance::*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    constexpr Return (Instance::*method_of(Return (Instance::*function)(Args...) const))(Args...) const
    {
        return function;
    }

    template<typename OutType, typename... Args>
    OutType mask_of(Args&&... args)
    {
        return (static_cast<OutType>(args) | ...);
    }
}// namespace Engine
