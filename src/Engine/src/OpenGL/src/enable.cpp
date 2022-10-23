#include <opengl_types.hpp>
#include <opengl_object.hpp>


API void api_enable(Engine::EnableCap cap)
{
    glEnable(_M_enable_caps.at(cap));
}

API void api_disable(Engine::EnableCap cap)
{
    glDisable(_M_enable_caps.at(cap));
}

API void api_blend_func(Engine::BlendFunc func1, Engine::BlendFunc func2)
{
    glBlendFunc(_M_blend_funcs.at(func1), _M_blend_funcs.at(func2));
}
