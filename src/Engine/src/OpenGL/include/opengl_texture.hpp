#pragma once
#include <opengl.hpp>
#include <opengl_object.hpp>
#include <Core/engine_types.hpp>

#define texture(id) object_of<OpenGL_Texture>(id)
#define make_texture(variable, id) OpenGL_Texture* variable = texture(id)

namespace Engine
{
    class OpenGL_Texture  : public OpenGL_Object{
    public:
        GLuint _M_ID;
        TextureParams _M_params;

        GLuint _M_GL_type;
        GLuint _M_GL_format;
        GLuint _M_GL_pixel_type;

        int _M_base_level = 0;
        DepthStencilMode _M_depth_stencil_mode = DepthStencilMode::DEPTH;
        CompareFunc _M_compare_func;
        CompareMode _M_compare_mode;
        int _M_max_lod_level = 1000;
        int _M_min_lod_level = -1000;
        int _M_max_mipmap_level = 1000;
        SwizzleRGBA _M_swizzle = {SwizzleRGBA::SwizzleValue::RED, SwizzleRGBA::SwizzleValue::GREEN,
                                  SwizzleRGBA::SwizzleValue::BLUE, SwizzleRGBA::SwizzleValue::ALPHA};
        WrapValue _M_wrap_s = WrapValue::REPEAT;
        WrapValue _M_wrap_t = WrapValue::REPEAT;
        WrapValue _M_wrap_r = WrapValue::REPEAT;

        void destroy() override;
        declare_hpp_destructor(OpenGL_Texture);
    };

}
