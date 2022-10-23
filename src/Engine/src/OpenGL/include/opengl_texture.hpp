#pragma once
#include <opengl.hpp>
#include <Core/engine_types.hpp>

#define texture(id) static_cast<OpenGL_Texture*>(object_of(id)->_M_data)
#define make_texture(variable, id) OpenGL_Texture* variable = texture(id)

namespace Engine
{
    struct OpenGL_Texture {
        GLuint _M_ID;
        TextureParams _M_params;

        GLuint _M_GL_type;
        GLuint _M_GL_format;
        GLuint _M_GL_pixel_type;

        std::size_t _M_references = 0;
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
    };

}
