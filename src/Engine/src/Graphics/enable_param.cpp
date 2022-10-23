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


}// namespace Engine
