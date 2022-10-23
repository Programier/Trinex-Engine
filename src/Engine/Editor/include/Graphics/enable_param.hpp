#pragma once

#include <Core/engine_types.hpp>
#include <TemplateFunctional/return_wrapper.hpp>
#include <functional>
#include <Core/export.hpp>

namespace Engine
{
    STRUCT EngineEnable {
        const EngineEnable& operator()(EnableCap cap) const;
    } enable;

    STRUCT EngineDisable {
        const EngineDisable& operator()(EnableCap cap) const;
    } disable;

    STRUCT EngineBlendFunc {
        const EngineBlendFunc& operator()(BlendFunc sfactor, BlendFunc dfactor) const;
    } blend_func;
}// namespace Engine
