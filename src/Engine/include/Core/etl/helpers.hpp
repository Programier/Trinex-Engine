#pragma once

namespace Engine
{

    template<typename Return, typename... Args>
    Return (*func_of(Return (*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*func_of(Return (Instance::*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*func_of(Return (Instance::*function)(Args...) const))(Args...) const
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*method_of(Return (Instance::*function)(Args...)))(Args...)
    {
        return function;
    }

    template<typename Return, typename Instance, typename... Args>
    Return (Instance::*method_of(Return (Instance::*function)(Args...) const))(Args...) const
    {
        return function;
    }

}// namespace Engine
