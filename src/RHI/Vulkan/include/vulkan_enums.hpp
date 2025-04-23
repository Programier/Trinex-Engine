#pragma once
#include <Core/enums.hpp>
#include <vulkan_headers.hpp>

namespace Engine::VulkanEnums
{
	constexpr inline vk::Format format_of(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::R8: return vk::Format::eR8Unorm;
			case ColorFormat::R8G8: return vk::Format::eR8G8Unorm;
			case ColorFormat::R8G8B8: return vk::Format::eR8G8B8Unorm;
			case ColorFormat::R8G8B8A8: return vk::Format::eR8G8B8A8Unorm;
			case ColorFormat::R8_SNORM: return vk::Format::eR8Snorm;
			case ColorFormat::R8G8_SNORM: return vk::Format::eR8G8Snorm;
			case ColorFormat::R8G8B8_SNORM: return vk::Format::eR8G8B8Snorm;
			case ColorFormat::R8G8B8A8_SNORM: return vk::Format::eR8G8B8A8Snorm;
			case ColorFormat::R8_UINT: return vk::Format::eR8Uint;
			case ColorFormat::R8G8_UINT: return vk::Format::eR8G8Uint;
			case ColorFormat::R8G8B8_UINT: return vk::Format::eR8G8B8Uint;
			case ColorFormat::R8G8B8A8_UINT: return vk::Format::eR8G8B8A8Uint;
			case ColorFormat::R8_SINT: return vk::Format::eR8Sint;
			case ColorFormat::R8G8_SINT: return vk::Format::eR8G8Sint;
			case ColorFormat::R8G8B8_SINT: return vk::Format::eR8G8B8Sint;
			case ColorFormat::R8G8B8A8_SINT: return vk::Format::eR8G8B8A8Sint;
			case ColorFormat::R16: return vk::Format::eR16Unorm;
			case ColorFormat::R16G16: return vk::Format::eR16G16Unorm;
			case ColorFormat::R16G16B16: return vk::Format::eR16G16B16Unorm;
			case ColorFormat::R16G16B16A16: return vk::Format::eR16G16B16A16Unorm;
			case ColorFormat::R16_SNORM: return vk::Format::eR16Snorm;
			case ColorFormat::R16G16_SNORM: return vk::Format::eR16G16Snorm;
			case ColorFormat::R16G16B16_SNORM: return vk::Format::eR16G16B16Snorm;
			case ColorFormat::R16G16B16A16_SNORM: return vk::Format::eR16G16B16A16Snorm;
			case ColorFormat::R16_UINT: return vk::Format::eR16Uint;
			case ColorFormat::R16G16_UINT: return vk::Format::eR16G16Uint;
			case ColorFormat::R16G16B16_UINT: return vk::Format::eR16G16B16Uint;
			case ColorFormat::R16G16B16A16_UINT: return vk::Format::eR16G16B16A16Uint;
			case ColorFormat::R16_SINT: return vk::Format::eR16Sint;
			case ColorFormat::R16G16_SINT: return vk::Format::eR16G16Sint;
			case ColorFormat::R16G16B16_SINT: return vk::Format::eR16G16B16Sint;
			case ColorFormat::R16G16B16A16_SINT: return vk::Format::eR16G16B16A16Sint;
			case ColorFormat::R32_UINT: return vk::Format::eR32Uint;
			case ColorFormat::R32G32_UINT: return vk::Format::eR32G32Uint;
			case ColorFormat::R32G32B32_UINT: return vk::Format::eR32G32B32Uint;
			case ColorFormat::R32G32B32A32_UINT: return vk::Format::eR32G32B32A32Uint;
			case ColorFormat::R32_SINT: return vk::Format::eR32Sint;
			case ColorFormat::R32G32_SINT: return vk::Format::eR32G32Sint;
			case ColorFormat::R32G32B32_SINT: return vk::Format::eR32G32B32Sint;
			case ColorFormat::R32G32B32A32_SINT: return vk::Format::eR32G32B32A32Sint;
			case ColorFormat::R16F: return vk::Format::eR16Sfloat;
			case ColorFormat::R16G16F: return vk::Format::eR16G16Sfloat;
			case ColorFormat::R16G16B16F: return vk::Format::eR16G16B16Sfloat;
			case ColorFormat::R16G16B16A16F: return vk::Format::eR16G16B16A16Sfloat;
			case ColorFormat::R32F: return vk::Format::eR32Sfloat;
			case ColorFormat::R32G32F: return vk::Format::eR32G32Sfloat;
			case ColorFormat::R32G32B32F: return vk::Format::eR32G32B32Sfloat;
			case ColorFormat::R32G32B32A32F: return vk::Format::eR32G32B32A32Sfloat;
			case ColorFormat::BC1_RGBA: return vk::Format::eBc1RgbaUnormBlock;
			case ColorFormat::BC2_RGBA: return vk::Format::eBc2UnormBlock;
			case ColorFormat::BC3_RGBA: return vk::Format::eBc3UnormBlock;
			case ColorFormat::BC4_R: return vk::Format::eBc4UnormBlock;
			case ColorFormat::BC5_RG: return vk::Format::eBc5UnormBlock;
			case ColorFormat::BC7_RGBA: return vk::Format::eBc7UnormBlock;
			case ColorFormat::ASTC_4x4_RGBA: return vk::Format::eAstc4x4UnormBlock;
			case ColorFormat::ASTC_6x6_RGBA: return vk::Format::eAstc6x6UnormBlock;
			case ColorFormat::ASTC_8x8_RGBA: return vk::Format::eAstc8x8UnormBlock;
			case ColorFormat::ASTC_10x10_RGBA: return vk::Format::eAstc10x10UnormBlock;
			case ColorFormat::ETC1_RGB: return vk::Format::eEtc2R8G8B8UnormBlock;
			case ColorFormat::ETC2_RGB: return vk::Format::eEtc2R8G8B8UnormBlock;
			case ColorFormat::ETC2_RGBA: return vk::Format::eEtc2R8G8B8A8UnormBlock;
			case ColorFormat::NV12: return vk::Format::eG8B8R82Plane420Unorm;
			case ColorFormat::P010: return vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16;
			case ColorFormat::Depth: return vk::Format::eD16Unorm;
			case ColorFormat::DepthStencil: return vk::Format::eD24UnormS8Uint;
			case ColorFormat::ShadowDepth: return vk::Format::eD16Unorm;

			default: return vk::Format::eUndefined;
		}
	}

	constexpr inline vk::ImageAspectFlags srv_aspect(vk::ImageAspectFlags aspect)
	{
		return aspect & (vk::ImageAspectFlagBits::eColor | vk::ImageAspectFlagBits::eDepth);
	}

	constexpr inline vk::ImageAspectFlags uav_aspect(vk::ImageAspectFlags aspect)
	{
		return aspect & (vk::ImageAspectFlagBits::eColor);
	}

	constexpr inline vk::ImageAspectFlags rtv_aspect(vk::ImageAspectFlags aspect)
	{
		return aspect & (vk::ImageAspectFlagBits::eColor);
	}

	constexpr inline vk::ImageAspectFlags dsv_aspect(vk::ImageAspectFlags aspect)
	{
		return aspect & (vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
	}

	constexpr inline vk::Filter filter_of(SamplerFilter filter)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
			case SamplerFilter::Trilinear: return vk::Filter::eLinear;
			default: return vk::Filter::eNearest;
		}
	}

	constexpr inline vk::BlendFactor blend_func_of(BlendFunc func, bool is_for_alpha)
	{
		switch (func)
		{
			case BlendFunc::Zero: return vk::BlendFactor::eZero;
			case BlendFunc::One: return vk::BlendFactor::eOne;
			case BlendFunc::SrcColor: return vk::BlendFactor::eSrcColor;
			case BlendFunc::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
			case BlendFunc::DstColor: return vk::BlendFactor::eDstColor;
			case BlendFunc::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
			case BlendFunc::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
			case BlendFunc::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
			case BlendFunc::DstAlpha: return vk::BlendFactor::eDstAlpha;
			case BlendFunc::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
			case BlendFunc::BlendFactor: return is_for_alpha ? vk::BlendFactor::eConstantAlpha : vk::BlendFactor::eConstantColor;
			case BlendFunc::OneMinusBlendFactor:
				return is_for_alpha ? vk::BlendFactor::eOneMinusConstantAlpha : vk::BlendFactor::eOneMinusConstantColor;

			default: return vk::BlendFactor::eZero;
		}
	}
}// namespace Engine::VulkanEnums
