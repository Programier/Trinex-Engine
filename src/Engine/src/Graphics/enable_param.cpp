#include <Graphics/enable_param.hpp>
#include <api_funcs.hpp>
#include <opengl.hpp>
#include <unordered_map>

namespace Engine
{
    const EngineEnable& EngineEnable::operator()(EnableCap cap) const
    {
        api_enable(cap);
        return *this;
    }

    const EngineDisable& EngineDisable::operator()(EnableCap cap) const
    {
        api_disable(cap);
        return *this;
    }

    const EngineBlendFunc& EngineBlendFunc::operator()(BlendFunc sfactor, BlendFunc dfactor) const
    {
        set_blend_func(sfactor, dfactor);
        return *this;
    }

    const EngineDepthFunc& EngineDepthFunc::operator()(CompareFunc func) const
    {
        set_depth_func(func);
        return *this;
    }

    const EngineDepthMask& EngineDepthMask::operator()(bool mask) const
    {
        set_depth_mask(mask);
        return *this;
    }


    const EngineStencilMask& EngineStencilMask::operator()(byte mask) const
    {
        set_stencil_mask(mask);
        return *this;
    }

    const EngineStencilOption& EngineStencilOption::operator()(StencilOption stencil_fail, StencilOption depth_fail,
                                                               StencilOption pass) const
    {
        set_stencil_option(stencil_fail, depth_fail, pass);
        return *this;
    }

    const EngineStencilFunc& EngineStencilFunc::operator()(Engine::CompareFunc func, int ref, byte mask) const
    {
        set_stencil_func(func, ref, mask);
        return *this;
    }


    ENGINE_EXPORT EngineEnable enable;
    ENGINE_EXPORT EngineDisable disable;
    ENGINE_EXPORT EngineBlendFunc blend_func;
    ENGINE_EXPORT EngineDepthFunc depth_func;
    ENGINE_EXPORT EngineDepthMask depth_mask;
    ENGINE_EXPORT EngineStencilMask stencil_mask;
    ENGINE_EXPORT EngineStencilOption stencil_option;
    ENGINE_EXPORT EngineStencilFunc stencil_func;
}// namespace Engine
