#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <Graphics/types/color_format.hpp>

namespace Engine
{
	trinex_implement_engine_enum(ColorFormat, Undefined, R8, R8G8, R8G8B8, R8G8B8A8, R8_SNORM, R8G8_SNORM, R8G8B8_SNORM,
	                             R8G8B8A8_SNORM, R8_UINT, R8G8_UINT, R8G8B8_UINT, R8G8B8A8_UINT, R8_SINT, R8G8_SINT, R8G8B8_SINT,
	                             R8G8B8A8_SINT, R16, R16G16, R16G16B16, R16G16B16A16, R16_SNORM, R16G16_SNORM, R16G16B16_SNORM,
	                             R16G16B16A16_SNORM, R16_UINT, R16G16_UINT, R16G16B16_UINT, R16G16B16A16_UINT, R16_SINT,
	                             R16G16_SINT, R16G16B16_SINT, R16G16B16A16_SINT, R32_UINT, R32G32_UINT, R32G32B32_UINT,
	                             R32G32B32A32_UINT, R32_SINT, R32G32_SINT, R32G32B32_SINT, R32G32B32A32_SINT, R16F, R16G16F,
	                             R16G16B16F, R16G16B16A16F, R32F, R32G32F, R32G32B32F, R32G32B32A32F, BC1_RGBA, BC2_RGBA,
	                             BC3_RGBA, BC4_R, BC5_RG, BC7_RGBA, ASTC_4x4_RGBA, ASTC_6x6_RGBA, ASTC_8x8_RGBA, ASTC_10x10_RGBA,
	                             ETC1_RGB, ETC2_RGB, ETC2_RGBA, NV12, P010, Depth, DepthStencil, ShadowDepth);

	trinex_implement_engine_enum(SurfaceFormat, Depth, ShadowDepth, R8, RG8, RGBA8, RG8B10A2, R16F, RG16F, RGBA16F);

	static ColorFormat::Capabilities s_capabilities[ColorFormat::static_count()] = {};

	ColorFormat::Capabilities ColorFormat::capabilities() const
	{
		return s_capabilities[value];
	}

	ColorFormat ColorFormat::add_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] |= capabilities;
		return *this;
	}

	ColorFormat ColorFormat::remove_capabilities(Capabilities capabilities) const
	{
		s_capabilities[value] &= ~capabilities;
		return *this;
	}

}// namespace Engine
