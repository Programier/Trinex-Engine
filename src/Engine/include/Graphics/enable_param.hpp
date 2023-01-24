#pragma once

#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <TemplateFunctional/return_wrapper.hpp>
#include <functional>

namespace Engine
{
    STRUCT EngineEnable
    {
        const EngineEnable& operator()(EnableCap cap) const;
    };

    STRUCT EngineDisable
    {
        const EngineDisable& operator()(EnableCap cap) const;
    };

    STRUCT EngineBlendFunc
    {
        const EngineBlendFunc& operator()(BlendFunc sfactor, BlendFunc dfactor) const;
    };

    STRUCT EngineDepthFunc
    {
        const EngineDepthFunc& operator()(CompareFunc func) const;
    };

    STRUCT EngineDepthMask
    {
        const EngineDepthMask& operator()(bool mask) const;
    };

    STRUCT EngineStencilMask
    {
        const EngineStencilMask& operator()(byte mask) const;
    };

    STRUCT EngineStencilOption
    {
        const EngineStencilOption& operator()(StencilOption stencil_fail, StencilOption depth_fail, StencilOption pass)
                const;
    };

    STRUCT EngineStencilFunc
    {
        const EngineStencilFunc& operator()(Engine::CompareFunc func, int ref, byte mask)
                const;
    };

    extern ENGINE_EXPORT EngineEnable enable;
    extern ENGINE_EXPORT EngineDisable disable;
    extern ENGINE_EXPORT EngineBlendFunc blend_func;
    extern ENGINE_EXPORT EngineDepthFunc depth_func;
    extern ENGINE_EXPORT EngineDepthMask depth_mask;
    extern ENGINE_EXPORT EngineStencilMask stencil_mask;
    extern ENGINE_EXPORT EngineStencilOption stencil_option;
    extern ENGINE_EXPORT EngineStencilFunc stencil_func;
}// namespace Engine
