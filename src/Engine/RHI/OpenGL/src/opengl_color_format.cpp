#include <Core/exception.hpp>
#include <opengl_color_format.hpp>

namespace Engine
{
    static const Map<ColorFormat, OpenGL_ColorFormat> _M_opengl_formats = {
            {ColorFormat::R8Unorm, {GL_R8, GL_RED, GL_UNSIGNED_BYTE}},
            {ColorFormat::R8Snorm, {GL_R8_SNORM, GL_RED, GL_BYTE}},
            {ColorFormat::S8Uint, {GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE}},
            {ColorFormat::R8Uint, {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE}},
            {ColorFormat::R8Sint, {GL_R8I, GL_RED_INTEGER, GL_BYTE}},
            {ColorFormat::R8G8Srgb, {}}, // TODO
            {ColorFormat::R8Uscaled, {}},// TODO
            {ColorFormat::R8Sscaled, {}},// TODO
            {ColorFormat::R16Unorm, {GL_R16_EXT, GL_RED, GL_UNSIGNED_SHORT}},
            {ColorFormat::D16Unorm, {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16Snorm, {GL_R16_EXT, GL_RED, GL_SHORT}},
            {ColorFormat::R16Uscaled, {}},// TODO
            {ColorFormat::R16Sscaled, {}},// TODO
            {ColorFormat::R16Uint, {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16Sint, {GL_R16I, GL_RED_INTEGER, GL_SHORT}},
            {ColorFormat::R16Sfloat, {GL_R16F, GL_RED, GL_HALF_FLOAT}},
            {ColorFormat::D32Sfloat, {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}},
            {ColorFormat::R32Sfloat, {GL_R32F, GL_RED, GL_FLOAT}},
            {ColorFormat::R32Uint, {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT}},
            {ColorFormat::R32Sint, {GL_R32I, GL_RED_INTEGER, GL_INT}},
            {ColorFormat::R8G8Unorm, {GL_RG8, GL_RG, GL_UNSIGNED_BYTE}},
            {ColorFormat::R8G8Snorm, {GL_RG8_SNORM, GL_RG, GL_BYTE}},
            {ColorFormat::R8G8Uscaled, {}},// TODO. Maybe {GL_RG8, GL_RG, GL_UNSIGNED_BYTE}?
            {ColorFormat::R8G8Sscaled, {}},// TODO. Maybe {GL_RG8_SNORM, GL_RG, GL_BYTE}?
            {ColorFormat::R8G8Uint, {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE}},
            {ColorFormat::R8G8Sint, {GL_RG8I, GL_RG_INTEGER, GL_BYTE}},
            {ColorFormat::R8G8Srgb, {}},// TODO
            {ColorFormat::R16G16Unorm, {GL_RG16_EXT, GL_RG, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16Snorm, {GL_RG16_SNORM_EXT, GL_RG_INTEGER, GL_SHORT}},
            {ColorFormat::R16G16Uscaled, {}},
            {ColorFormat::R16G16Sscaled, {}},
            {ColorFormat::R16G16Uint, {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16Sint, {GL_RG16I, GL_RG_INTEGER, GL_SHORT}},
            {ColorFormat::R16G16Sfloat, {GL_RG16F, GL_RG, GL_HALF_FLOAT}},
            {ColorFormat::D16UnormS8Uint, {}},// TODO
            {ColorFormat::D24UnormS8Uint, {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8}},
            {ColorFormat::R32G32Uint, {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT}},
            {ColorFormat::R32G32Sint, {GL_RG32I, GL_RG_INTEGER, GL_INT}},
            {ColorFormat::R32G32Sfloat, {GL_RG32F, GL_RG, GL_FLOAT}},
            {ColorFormat::D32SfloatS8Uint, {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV}},
            {ColorFormat::R8G8B8Unorm, {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8Unorm, {}},// TODO
            {ColorFormat::R8G8B8Snorm, {GL_RGB8_SNORM, GL_RGB, GL_BYTE}},
            {ColorFormat::B8G8R8Snorm, {}},  // TODO
            {ColorFormat::R8G8B8Uscaled, {}},// TODO
            {ColorFormat::B8G8R8Uscaled, {}},// TODO
            {ColorFormat::R8G8B8Sscaled, {GL_RGB8_SNORM, GL_RGB, GL_BYTE}},
            {ColorFormat::B8G8R8Sscaled, {}},// TODO
            {ColorFormat::R8G8B8Uint, {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8Uint, {}},// TODO
            {ColorFormat::R8G8B8Sint, {GL_RGB8I, GL_RGB_INTEGER, GL_BYTE}},
            {ColorFormat::B8G8R8Sint, {}},// TODO
            {ColorFormat::R8G8B8Srgb, {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8Srgb, {}},// TODO
            {ColorFormat::R16G16B16Unorm, {GL_RGB16_EXT, GL_RGB, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16B16Snorm, {GL_RGB16_SNORM_EXT, GL_RGB, GL_SHORT}},
            {ColorFormat::R16G16B16Uscaled, {}},// TODO
            {ColorFormat::R16G16B16Sscaled, {}},// TODO
            {ColorFormat::R16G16B16Uint, {GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16B16Sint, {GL_RGB16I, GL_RGB_INTEGER, GL_SHORT}},
            {ColorFormat::R16G16B16Sfloat, {GL_RGB16F, GL_RGB, GL_HALF_FLOAT}},
            {ColorFormat::R32G32B32Uint, {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT}},
            {ColorFormat::R32G32B32Sint, {GL_RGB32I, GL_RGB_INTEGER, GL_INT}},
            {ColorFormat::R32G32B32Sfloat, {GL_RGB32F, GL_RGB, GL_FLOAT}},
            {ColorFormat::R8G8B8A8Unorm, {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8A8Unorm, {}},// TODO
            {ColorFormat::R8G8B8A8Snorm, {GL_RGBA8_SNORM, GL_RGBA, GL_BYTE}},
            {ColorFormat::B8G8R8A8Snorm, {}},// TODO
            {ColorFormat::R8G8B8A8Uscaled, {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8A8Uscaled, {}},// TODO
            {ColorFormat::R8G8B8A8Sscaled, {GL_RGBA8_SNORM, GL_RGBA, GL_BYTE}},
            {ColorFormat::B8G8R8A8Sscaled, {}},// TODO
            {ColorFormat::R8G8B8A8Uint, {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8A8Uint, {}},// TODO
            {ColorFormat::R8G8B8A8Sint, {GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE}},
            {ColorFormat::B8G8R8A8Sint, {}},// TODO
            {ColorFormat::R8G8B8A8Srgb, {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE}},
            {ColorFormat::B8G8R8A8Srgb, {}},// TODO
            {ColorFormat::R16G16B16A16Unorm, {GL_RGBA16_EXT, GL_RGBA, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16B16A16Snorm, {GL_RGBA16_SNORM_EXT, GL_RGBA, GL_SHORT}},
            {ColorFormat::R16G16B16A16Uscaled, {}},// TODO
            {ColorFormat::R16G16B16A16Sscaled, {}},// TODO
            {ColorFormat::R16G16B16A16Uint, {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT}},
            {ColorFormat::R16G16B16A16Sint, {GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT}},
            {ColorFormat::R16G16B16A16Sfloat, {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT}},
            {ColorFormat::R32G32B32A32Uint, {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT}},
            {ColorFormat::R32G32B32A32Sint, {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT}},
            {ColorFormat::R32G32B32A32Sfloat, {GL_RGBA32F, GL_RGBA, GL_FLOAT}},
    };


    static FORCE_INLINE bool is_valid(const OpenGL_ColorFormat& info)
    {
        return info.format != 0 && info.internal_format != 0 && info.type != 0;
    }

    OpenGL_ColorFormat OpenGL_ColorFormat::from(ColorFormat format)
    {
        auto it = _M_opengl_formats.find(format);
        if (it == _M_opengl_formats.end() || !is_valid(it->second))
        {
            throw EngineException("Invalid color format");
        }
        return it->second;
    }
}// namespace Engine
