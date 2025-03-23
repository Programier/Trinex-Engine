#pragma once
#include <Core/enums.hpp>
#include <vulkan_headers.hpp>

namespace Engine::VulkanEnums
{
	constexpr inline vk::Format from_color_format(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::Undefined:
				return vk::Format::eUndefined;
			case ColorFormat::FloatR:
				return vk::Format::eR16Sfloat;
			case ColorFormat::FloatRGBA:
				return vk::Format::eR16G16B16A16Sfloat;
			case ColorFormat::R8:
				return vk::Format::eR8Unorm;
			case ColorFormat::R8G8B8A8:
				return vk::Format::eR8G8B8A8Unorm;
			case ColorFormat::DepthStencil:
				return vk::Format::eD24UnormS8Uint;
			case ColorFormat::ShadowDepth:
				return vk::Format::eD32Sfloat;
			case ColorFormat::Depth:
				return vk::Format::eD32Sfloat;
			case ColorFormat::BC1:
				return vk::Format::eBc1RgbaUnormBlock;
			case ColorFormat::BC2:
				return vk::Format::eBc2UnormBlock;
			case ColorFormat::BC3:
				return vk::Format::eBc3UnormBlock;

			default:
				return vk::Format::eUndefined;
		}
	}

	constexpr inline ColorFormat to_color_format(vk::Format format)
	{
		switch (format)
		{
			case vk::Format::eUndefined:
				return ColorFormat::Undefined;
			case vk::Format::eR16Sfloat:
				return ColorFormat::FloatR;
			case vk::Format::eR16G16B16A16Sfloat:
				return ColorFormat::FloatRGBA;
			case vk::Format::eR8Unorm:
				return ColorFormat::R8;
			case vk::Format::eR8G8B8A8Unorm:
				return ColorFormat::R8G8B8A8;
			case vk::Format::eD24UnormS8Uint:
				return ColorFormat::DepthStencil;
			case vk::Format::eD32Sfloat:
				return ColorFormat::ShadowDepth;
			case vk::Format::eBc1RgbaUnormBlock:
				return ColorFormat::BC1;
			case vk::Format::eBc2UnormBlock:
				return ColorFormat::BC2;
			case vk::Format::eBc3UnormBlock:
				return ColorFormat::BC3;
			default:
				return ColorFormat::Undefined;
		}

		return ColorFormat::Undefined;
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
}// namespace Engine::VulkanEnums
