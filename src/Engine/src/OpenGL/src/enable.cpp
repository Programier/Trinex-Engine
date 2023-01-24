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

API void api_set_depth_mask(bool flag)
{
    glDepthMask(static_cast<GLboolean>(flag));
}

API void api_set_stencil_mask(byte mask)
{
    glStencilMask(mask);
}

API void api_set_stencil_func(Engine::CompareFunc func, int ref, byte mask)
{
    glStencilFunc(_M_compare_funcs.at(func), ref, mask);
}

API void api_set_stencil_option(Engine::StencilOption stencil_fail, Engine::StencilOption depth_fail,
                                Engine::StencilOption pass)
{
    glStencilOp(_M_stencil_options.at(stencil_fail), _M_stencil_options.at(depth_fail), _M_stencil_options.at(pass));
}
