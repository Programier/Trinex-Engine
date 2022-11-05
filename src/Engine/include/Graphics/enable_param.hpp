#pragma once

#include <Core/engine_types.hpp>
#include <TemplateFunctional/return_wrapper.hpp>
#include <functional>
#include <Core/export.hpp>

namespace Engine
{
    STRUCT EngineEnable {
        const EngineEnable& operator()(EnableCap cap) const;
    };

    STRUCT EngineDisable {
        const EngineDisable& operator()(EnableCap cap) const;
    };

    STRUCT EngineBlendFunc {
        const EngineBlendFunc& operator()(BlendFunc sfactor, BlendFunc dfactor) const;
    };

    STRUCT EngineDepthFunc {
        const EngineDepthFunc& operator()(CompareFunc func) const;
    };

    extern ENGINE_EXPORT EngineEnable enable;
    extern ENGINE_EXPORT EngineDisable disable;
    extern ENGINE_EXPORT EngineBlendFunc blend_func;
    extern ENGINE_EXPORT EngineDepthFunc depth_func;
}// namespace Engine
