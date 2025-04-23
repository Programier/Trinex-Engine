#include <opengl_api.hpp>
#include <opengl_color_format.hpp>

namespace Engine
{
	OpenGL_ColorInfo color_format_from_engine_format(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::R8: return OpenGL_ColorInfo(GL_R8, GL_RED, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8: return OpenGL_ColorInfo(GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8B8: return OpenGL_ColorInfo(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8B8A8: return OpenGL_ColorInfo(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
			case ColorFormat::R8_SNORM: return OpenGL_ColorInfo(GL_R8_SNORM, GL_RED, GL_BYTE);
			case ColorFormat::R8G8_SNORM: return OpenGL_ColorInfo(GL_RG8_SNORM, GL_RG, GL_BYTE);
			case ColorFormat::R8G8B8_SNORM: return OpenGL_ColorInfo(GL_RGB8_SNORM, GL_RGB, GL_BYTE);
			case ColorFormat::R8G8B8A8_SNORM: return OpenGL_ColorInfo(GL_RGBA8_SNORM, GL_RGBA, GL_BYTE);
			case ColorFormat::R8_UINT: return OpenGL_ColorInfo(GL_R8UI, GL_RED, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8_UINT: return OpenGL_ColorInfo(GL_RG8UI, GL_RG, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8B8_UINT: return OpenGL_ColorInfo(GL_RGB8UI, GL_RGB, GL_UNSIGNED_BYTE);
			case ColorFormat::R8G8B8A8_UINT: return OpenGL_ColorInfo(GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_BYTE);
			case ColorFormat::R8_SINT: return OpenGL_ColorInfo(GL_R8I, GL_RED, GL_BYTE);
			case ColorFormat::R8G8_SINT: return OpenGL_ColorInfo(GL_RG8I, GL_RG, GL_BYTE);
			case ColorFormat::R8G8B8_SINT: return OpenGL_ColorInfo(GL_RGB8I, GL_RGB, GL_BYTE);
			case ColorFormat::R8G8B8A8_SINT: return OpenGL_ColorInfo(GL_RGBA8I, GL_RGBA, GL_BYTE);
			case ColorFormat::R16: return OpenGL_ColorInfo(GL_R16, GL_RED, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16: return OpenGL_ColorInfo(GL_RG16, GL_RG, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16B16: return OpenGL_ColorInfo(GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16B16A16: return OpenGL_ColorInfo(GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT);
			case ColorFormat::R16_SNORM: return OpenGL_ColorInfo(GL_R16_SNORM, GL_RED, GL_SHORT);
			case ColorFormat::R16G16_SNORM: return OpenGL_ColorInfo(GL_RG16_SNORM, GL_RG, GL_SHORT);
			case ColorFormat::R16G16B16_SNORM: return OpenGL_ColorInfo(GL_RGB16_SNORM, GL_RGB, GL_SHORT);
			case ColorFormat::R16G16B16A16_SNORM: return OpenGL_ColorInfo(GL_RGBA16_SNORM, GL_RGBA, GL_SHORT);
			case ColorFormat::R16_UINT: return OpenGL_ColorInfo(GL_R16UI, GL_RED, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16_UINT: return OpenGL_ColorInfo(GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16B16_UINT: return OpenGL_ColorInfo(GL_RGB16UI, GL_RGB, GL_UNSIGNED_SHORT);
			case ColorFormat::R16G16B16A16_UINT: return OpenGL_ColorInfo(GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT);
			case ColorFormat::R16_SINT: return OpenGL_ColorInfo(GL_R16I, GL_RED, GL_SHORT);
			case ColorFormat::R16G16_SINT: return OpenGL_ColorInfo(GL_RG16I, GL_RG, GL_SHORT);
			case ColorFormat::R16G16B16_SINT: return OpenGL_ColorInfo(GL_RGB16I, GL_RGB, GL_SHORT);
			case ColorFormat::R16G16B16A16_SINT: return OpenGL_ColorInfo(GL_RGBA16I, GL_RGBA, GL_SHORT);
			case ColorFormat::R32_UINT: return OpenGL_ColorInfo(GL_R32UI, GL_RED, GL_UNSIGNED_INT);
			case ColorFormat::R32G32_UINT: return OpenGL_ColorInfo(GL_RG32UI, GL_RG, GL_UNSIGNED_INT);
			case ColorFormat::R32G32B32_UINT: return OpenGL_ColorInfo(GL_RGB32UI, GL_RGB, GL_UNSIGNED_INT);
			case ColorFormat::R32G32B32A32_UINT: return OpenGL_ColorInfo(GL_RGBA32UI, GL_RGBA, GL_UNSIGNED_INT);
			case ColorFormat::R32_SINT: return OpenGL_ColorInfo(GL_R32I, GL_RED, GL_INT);
			case ColorFormat::R32G32_SINT: return OpenGL_ColorInfo(GL_RG32I, GL_RG, GL_INT);
			case ColorFormat::R32G32B32_SINT: return OpenGL_ColorInfo(GL_RGB32I, GL_RGB, GL_INT);
			case ColorFormat::R32G32B32A32_SINT: return OpenGL_ColorInfo(GL_RGBA32I, GL_RGBA, GL_INT);
			case ColorFormat::R16F: return OpenGL_ColorInfo(GL_R16F, GL_RED, GL_HALF_FLOAT);
			case ColorFormat::R16G16F: return OpenGL_ColorInfo(GL_RG16F, GL_RG, GL_HALF_FLOAT);
			case ColorFormat::R16G16B16F: return OpenGL_ColorInfo(GL_RGB16F, GL_RGB, GL_HALF_FLOAT);
			case ColorFormat::R16G16B16A16F: return OpenGL_ColorInfo(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
			case ColorFormat::R32F: return OpenGL_ColorInfo(GL_R32F, GL_RED, GL_FLOAT);
			case ColorFormat::R32G32F: return OpenGL_ColorInfo(GL_RG32F, GL_RG, GL_FLOAT);
			case ColorFormat::R32G32B32F: return OpenGL_ColorInfo(GL_RGB32F, GL_RGB, GL_FLOAT);
			case ColorFormat::R32G32B32A32F: return OpenGL_ColorInfo(GL_RGBA32F, GL_RGBA, GL_FLOAT);
			case ColorFormat::DepthStencil:
				return OpenGL_ColorInfo(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
			case ColorFormat::ShadowDepth: return OpenGL_ColorInfo(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
			case ColorFormat::Depth: return OpenGL_ColorInfo(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
			case ColorFormat::BC1_RGBA: return OpenGL_ColorInfo(0x83F1, 0, 0);
			case ColorFormat::BC2_RGBA: break;
			case ColorFormat::BC3_RGBA: return OpenGL_ColorInfo(0x8C4F, 0, 0);
			default: break;
		}
		return OpenGL_ColorInfo(0, 0, 0);
	}
}// namespace Engine
