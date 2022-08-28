#pragma once

#include <BasicFunctional/return_wrapper.hpp>
#include <functional>

namespace Engine
{
    enum class EnableCap : unsigned int
    {
        Blend = 0,
        DepthTest = 1,
        CullFace = 2
    };

    enum class BlendFunc : unsigned int
    {
        Zero,
        One,
        SrcColor,
        OneMinusScrColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };


    extern const ReturnFunctionWrapper<void, const BlendFunc&, const BlendFunc&> blend_func;
    extern const ReturnFunctionWrapper<void, const EnableCap&> enable;
    extern const ReturnFunctionWrapper<void, const EnableCap&> disable;
}// namespace Engine
