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

    ENGINE_EXPORT EngineEnable enable;
    ENGINE_EXPORT EngineDisable disable;
    ENGINE_EXPORT EngineBlendFunc blend_func;
    ENGINE_EXPORT EngineDepthFunc depth_func;
}// namespace Engine
