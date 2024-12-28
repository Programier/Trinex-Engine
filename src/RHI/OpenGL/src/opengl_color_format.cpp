#include <opengl_api.hpp>
#include <opengl_color_format.hpp>

namespace Engine
{
	OpenGL_ColorInfo color_format_from_engine_format(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::FloatR:
				return OpenGL_ColorInfo(GL_R32F, GL_RED, GL_FLOAT);
			case ColorFormat::FloatRGBA:
				return OpenGL_ColorInfo(GL_RGBA32F, GL_RGBA, GL_FLOAT);
			case ColorFormat::R8:
				return OpenGL_ColorInfo(GL_R8, GL_RED, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8B8A8:
				return OpenGL_ColorInfo(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
			case ColorFormat::DepthStencil:
				return OpenGL_ColorInfo(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
			case ColorFormat::ShadowDepth:
				return OpenGL_ColorInfo(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
			case ColorFormat::Depth:
				return OpenGL_ColorInfo(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
			case ColorFormat::BC1:
				return OpenGL_ColorInfo(0x83F1, 0, 0);
			case ColorFormat::BC2:
				break;
			case ColorFormat::BC3:
				return OpenGL_ColorInfo(0x8C4F, 0, 0);
			default:
				break;
		}
		return OpenGL_ColorInfo(0, 0, 0);
	}
}// namespace Engine
