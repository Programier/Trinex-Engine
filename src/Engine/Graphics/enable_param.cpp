#include <GL/glew.h>
#include <Graphics/enable_param.hpp>
#include <unordered_map>


namespace Engine
{

    static const std::unordered_map<EnableCap, int> OpenGL_EnableCaps = {{EnableCap::AlphaTest, GL_ALPHA_TEST},
                                                                         {EnableCap::AutoNormal, GL_AUTO_NORMAL},
                                                                         {EnableCap::Blend, GL_BLEND},
                                                                         {EnableCap::DepthTest, GL_DEPTH_TEST},
                                                                         {EnableCap::CullFace, GL_CULL_FACE}};

    static const std::unordered_map<BlendFunc, int> OpenGL_BlendFuncs = {
            {BlendFunc::Zero, GL_ZERO},
            {BlendFunc::One, GL_ONE},
            {BlendFunc::SrcColor, GL_SRC_COLOR},
            {BlendFunc::OneMinusScrColor, GL_ONE_MINUS_SRC_COLOR},
            {BlendFunc::DstColor, GL_DST_COLOR},
            {BlendFunc::OneMinusDstColor, GL_ONE_MINUS_DST_COLOR},
            {BlendFunc::SrcAlpha, GL_SRC_ALPHA},
            {BlendFunc::OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA},
            {BlendFunc::DstAlpha, GL_DST_ALPHA},
            {BlendFunc::OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA},
            {BlendFunc::ConstantColor, GL_CONSTANT_COLOR},
            {BlendFunc::OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR},
            {BlendFunc::ConstantAlpha, GL_CONSTANT_ALPHA},
            {BlendFunc::OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA}};

    void blend_function(const BlendFunc& s_factor, const BlendFunc& d_factor)
    {
        glBlendFunc(OpenGL_BlendFuncs.at(s_factor), OpenGL_BlendFuncs.at(d_factor));
    }

    static void enable_func(const EnableCap& cap)
    {
        glEnable(OpenGL_EnableCaps.at(cap));
    }

    static void disable_func(const EnableCap& cap)
    {
        glDisable(OpenGL_EnableCaps.at(cap));
    }

    const ReturnFunctionWrapper<void, const EnableCap&> enable(enable_func);
    const ReturnFunctionWrapper<void, const EnableCap&> disable(enable_func);
    const ReturnFunctionWrapper<void, const BlendFunc&, const BlendFunc&> blend_func(blend_function);
}// namespace Engine
