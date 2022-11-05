#include <opengl_object.hpp>
#include <opengl_types.hpp>


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

API void api_set_depth_func(Engine::CompareFunc func)
{
    glDepthFunc(_M_compare_funcs.at(func));
}
