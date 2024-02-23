#include <opengl_api.hpp>
#include <opengl_color_format.hpp>

namespace Engine
{
    OpenGL_ColorInfo color_format_from_engine_format(ColorFormat format)
    {

        switch (format)
        {
            case ColorFormat::R8Unorm:
                return OpenGL_ColorInfo(GL_R8, GL_RED, GL_UNSIGNED_BYTE);
            case ColorFormat::R8Snorm:
                return OpenGL_ColorInfo(GL_R8_SNORM, GL_RED, GL_BYTE);
            case ColorFormat::S8Uint:
                return OpenGL_ColorInfo(GL_STENCIL_INDEX8, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE);
            case ColorFormat::R8Uint:
                return OpenGL_ColorInfo(GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE);
            case ColorFormat::R8Sint:
                return OpenGL_ColorInfo(GL_R8I, GL_RED_INTEGER, GL_BYTE);
            case ColorFormat::R8G8Srgb:
                break;// TODO
            case ColorFormat::R8Uscaled:
                break;// TODO
            case ColorFormat::R8Sscaled:
                break;// TODO
            case ColorFormat::R16Unorm:
                return OpenGL_ColorInfo(GL_R16UI, GL_RED, GL_UNSIGNED_SHORT);
            case ColorFormat::D16Unorm:
                return OpenGL_ColorInfo(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT);
            case ColorFormat::R16Snorm:
                return OpenGL_ColorInfo(GL_R16I, GL_RED, GL_SHORT);
            case ColorFormat::R16Uscaled:
                break;// TODO
            case ColorFormat::R16Sscaled:
                break;// TODO
            case ColorFormat::R16Uint:
                return OpenGL_ColorInfo(GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT);
            case ColorFormat::R16Sint:
                return OpenGL_ColorInfo(GL_R16I, GL_RED_INTEGER, GL_SHORT);
            case ColorFormat::R16Sfloat:
                return OpenGL_ColorInfo(GL_R16F, GL_RED, GL_HALF_FLOAT);
            case ColorFormat::D32Sfloat:
                return OpenGL_ColorInfo(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
            case ColorFormat::R32Sfloat:
                return OpenGL_ColorInfo(GL_R32F, GL_RED, GL_FLOAT);
            case ColorFormat::R32Uint:
                return OpenGL_ColorInfo(GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
            case ColorFormat::R32Sint:
                return OpenGL_ColorInfo(GL_R32I, GL_RED_INTEGER, GL_INT);
            case ColorFormat::R8G8Unorm:
                return OpenGL_ColorInfo(GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
            case ColorFormat::R8G8Snorm:
                return OpenGL_ColorInfo(GL_RG8_SNORM, GL_RG, GL_BYTE);
            case ColorFormat::R8G8Uscaled:
                break;// TODO. Maybe OpenGL_ColorInfo(GL_RG8, GL_RG, GL_UNSIGNED_BYTE}?
            case ColorFormat::R8G8Sscaled:
                break;// TODO. Maybe OpenGL_ColorInfo(GL_RG8_SNORM, GL_RG, GL_BYTE}?
            case ColorFormat::R8G8Uint:
                return OpenGL_ColorInfo(GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE);
            case ColorFormat::R8G8Sint:
                return OpenGL_ColorInfo(GL_RG8I, GL_RG_INTEGER, GL_BYTE);
            case ColorFormat::R16G16Unorm:
                return OpenGL_ColorInfo(GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT);

            case ColorFormat::R16G16Snorm:
#if USING_OPENGL_CORE
                return OpenGL_ColorInfo(GL_RG16_SNORM, GL_RG_INTEGER, GL_SHORT);
#else
                return OpenGL_ColorInfo(GL_RG16I, GL_RG_INTEGER, GL_SHORT);
#endif
            case ColorFormat::R16G16Uscaled:
                break;
            case ColorFormat::R16G16Sscaled:
                break;
            case ColorFormat::R16G16Uint:
                return OpenGL_ColorInfo(GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT);
            case ColorFormat::R16G16Sint:
                return OpenGL_ColorInfo(GL_RG16I, GL_RG_INTEGER, GL_SHORT);
            case ColorFormat::R16G16Sfloat:
                return OpenGL_ColorInfo(GL_RG16F, GL_RG, GL_HALF_FLOAT);
            case ColorFormat::D16UnormS8Uint:
                break;// TODO
            case ColorFormat::D24UnormS8Uint:
                return OpenGL_ColorInfo(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
            case ColorFormat::R32G32Uint:
                return OpenGL_ColorInfo(GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT);
            case ColorFormat::R32G32Sint:
                return OpenGL_ColorInfo(GL_RG32I, GL_RG_INTEGER, GL_INT);
            case ColorFormat::R32G32Sfloat:
                return OpenGL_ColorInfo(GL_RG32F, GL_RG, GL_FLOAT);
            case ColorFormat::D32SfloatS8Uint:
                return OpenGL_ColorInfo(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
            case ColorFormat::R8G8B8Unorm:
                return OpenGL_ColorInfo(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8Unorm:
                break;// TODO
            case ColorFormat::R8G8B8Snorm:
                return OpenGL_ColorInfo(GL_RGB8_SNORM, GL_RGB, GL_BYTE);
            case ColorFormat::B8G8R8Snorm:
                break;// TODO
            case ColorFormat::R8G8B8Uscaled:
                break;// TODO
            case ColorFormat::B8G8R8Uscaled:
                break;// TODO
            case ColorFormat::R8G8B8Sscaled:
                return OpenGL_ColorInfo(GL_RGB8_SNORM, GL_RGB, GL_BYTE);
            case ColorFormat::B8G8R8Sscaled:
                break;// TODO
            case ColorFormat::R8G8B8Uint:
                return OpenGL_ColorInfo(GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8Uint:
                break;// TODO
            case ColorFormat::R8G8B8Sint:
                return OpenGL_ColorInfo(GL_RGB8I, GL_RGB_INTEGER, GL_BYTE);
            case ColorFormat::B8G8R8Sint:
                break;// TODO
            case ColorFormat::R8G8B8Srgb:
                return OpenGL_ColorInfo(GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8Srgb:
                break;// TODO
            case ColorFormat::R16G16B16Unorm:
                return OpenGL_ColorInfo(GL_RGB16UI, GL_RGB, GL_UNSIGNED_SHORT);
            case ColorFormat::R16G16B16Snorm:
#if USING_OPENGL_CORE
                return OpenGL_ColorInfo(GL_RGB16_SNORM, GL_RGB, GL_SHORT);
#else
                return OpenGL_ColorInfo(GL_RGB16I, GL_RGB, GL_SHORT);
#endif
            case ColorFormat::R16G16B16Uscaled:
                break;// TODO
            case ColorFormat::R16G16B16Sscaled:
                break;// TODO
            case ColorFormat::R16G16B16Uint:
                return OpenGL_ColorInfo(GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT);
            case ColorFormat::R16G16B16Sint:
                return OpenGL_ColorInfo(GL_RGB16I, GL_RGB_INTEGER, GL_SHORT);
            case ColorFormat::R16G16B16Sfloat:
                return OpenGL_ColorInfo(GL_RGB16F, GL_RGB, GL_HALF_FLOAT);
            case ColorFormat::R32G32B32Uint:
                return OpenGL_ColorInfo(GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT);
            case ColorFormat::R32G32B32Sint:
                return OpenGL_ColorInfo(GL_RGB32I, GL_RGB_INTEGER, GL_INT);
            case ColorFormat::R32G32B32Sfloat:
                return OpenGL_ColorInfo(GL_RGB32F, GL_RGB, GL_FLOAT);
            case ColorFormat::R8G8B8A8Unorm:
                return OpenGL_ColorInfo(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8A8Unorm:
                break;// TODO
            case ColorFormat::R8G8B8A8Snorm:
                return OpenGL_ColorInfo(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE);
            case ColorFormat::B8G8R8A8Snorm:
                break;// TODO
            case ColorFormat::R8G8B8A8Uscaled:
                return OpenGL_ColorInfo(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8A8Uscaled:
                break;// TODO
            case ColorFormat::R8G8B8A8Sscaled:
                return OpenGL_ColorInfo(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE);
            case ColorFormat::B8G8R8A8Sscaled:
                break;// TODO
            case ColorFormat::R8G8B8A8Uint:
                return OpenGL_ColorInfo(GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8A8Uint:
                break;// TODO
            case ColorFormat::R8G8B8A8Sint:
                return OpenGL_ColorInfo(GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE);
            case ColorFormat::B8G8R8A8Sint:
                break;// TODO
            case ColorFormat::R8G8B8A8Srgb:
                return OpenGL_ColorInfo(GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE);
            case ColorFormat::B8G8R8A8Srgb:
                break;// TODO
            case ColorFormat::R16G16B16A16Unorm:
                return OpenGL_ColorInfo(GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT);
            case ColorFormat::R16G16B16A16Snorm:
#if USING_OPENGL_CORE
                return OpenGL_ColorInfo(GL_RGBA16_SNORM, GL_RGBA, GL_SHORT);
#else
                return OpenGL_ColorInfo(GL_RGBA16I, GL_RGBA, GL_SHORT);
#endif
            case ColorFormat::R16G16B16A16Uscaled:
                break;// TODO
            case ColorFormat::R16G16B16A16Sscaled:
                break;// TODO
            case ColorFormat::R16G16B16A16Uint:
                return OpenGL_ColorInfo(GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT);
            case ColorFormat::R16G16B16A16Sint:
                return OpenGL_ColorInfo(GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT);
            case ColorFormat::R16G16B16A16Sfloat:
                return OpenGL_ColorInfo(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
            case ColorFormat::R32G32B32A32Uint:
                return OpenGL_ColorInfo(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
            case ColorFormat::R32G32B32A32Sint:
                return OpenGL_ColorInfo(GL_RGBA32I, GL_RGBA_INTEGER, GL_INT);
            case ColorFormat::R32G32B32A32Sfloat:
                return OpenGL_ColorInfo(GL_RGBA32F, GL_RGBA, GL_FLOAT);
            case ColorFormat::BC7Unorm:
                return OpenGL_ColorInfo(0x8E8C, 0, 0);

            default:
                return OpenGL_ColorInfo(0, 0, 0);
        }
        return OpenGL_ColorInfo(0, 0, 0);
    }

    ColorFormatFeatures OpenGL::color_format_features(ColorFormat format)
    {
        OpenGL_ColorInfo info = color_format_from_engine_format(format);
        ColorFormatFeatures features;

        if (info.is_valid())
        {
            ColorFormatAspect aspect          = ColorFormatInfo::info_of(format).aspect();
            features.is_supported             = true;
            features.support_color_attachment = aspect == ColorFormatAspect::Color;
            features.support_depth_stencil    = aspect != ColorFormatAspect::Color && aspect != ColorFormatAspect::None;
        }

        return features;
    }


    static struct OpenGLColorFormats {
        ColorFormat base_color           = ColorFormat::R8G8B8A8Unorm;
        ColorFormat position_format      = ColorFormat::R16G16B16A16Sfloat;
        ColorFormat normal_format        = ColorFormat::R16G16B16A16Sfloat;
        ColorFormat emissive_format      = ColorFormat::R8G8B8A8Unorm;
        ColorFormat data_format          = ColorFormat::R8G8B8A8Unorm;
        ColorFormat depth_format         = ColorFormat::D32Sfloat;
        ColorFormat stencil_format       = ColorFormat::S8Uint;
        ColorFormat depth_stencil_format = ColorFormat::D32SfloatS8Uint;
    } formats;

    ColorFormat OpenGL::base_color_format()
    {
        return formats.base_color;
    }

    ColorFormat OpenGL::position_format()
    {
        return formats.position_format;
    }

    ColorFormat OpenGL::normal_format()
    {
        return formats.normal_format;
    }

    ColorFormat OpenGL::emissive_format()
    {
        return formats.emissive_format;
    }

    ColorFormat OpenGL::data_buffer_format()
    {
        return formats.data_format;
    }

    ColorFormat OpenGL::depth_format()
    {
        return formats.depth_format;
    }

    ColorFormat OpenGL::stencil_format()
    {
        return formats.stencil_format;
    }

    ColorFormat OpenGL::depth_stencil_format()
    {
        return formats.depth_stencil_format;
    }
}// namespace Engine
