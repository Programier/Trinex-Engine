#pragma once
#include <Core/enums.hpp>
#include <Graphics/types/color_format.hpp>
#include <Graphics/types/rhi_access.hpp>
#include <vulkan_headers.hpp>

namespace Engine::VulkanEnums
{
	constexpr inline vk::Format format_of(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::R8: return vk::Format::eR8Unorm;
			case ColorFormat::R8G8: return vk::Format::eR8G8Unorm;
			case ColorFormat::R8G8B8A8: return vk::Format::eR8G8B8A8Unorm;
			case ColorFormat::R8_SNORM: return vk::Format::eR8Snorm;
			case ColorFormat::R8G8_SNORM: return vk::Format::eR8G8Snorm;
			case ColorFormat::R8G8B8A8_SNORM: return vk::Format::eR8G8B8A8Snorm;
			case ColorFormat::R8_UINT: return vk::Format::eR8Uint;
			case ColorFormat::R8G8_UINT: return vk::Format::eR8G8Uint;
			case ColorFormat::R8G8B8A8_UINT: return vk::Format::eR8G8B8A8Uint;
			case ColorFormat::R8_SINT: return vk::Format::eR8Sint;
			case ColorFormat::R8G8_SINT: return vk::Format::eR8G8Sint;
			case ColorFormat::R8G8B8A8_SINT: return vk::Format::eR8G8B8A8Sint;
			case ColorFormat::R16: return vk::Format::eR16Unorm;
			case ColorFormat::R16G16: return vk::Format::eR16G16Unorm;
			case ColorFormat::R16G16B16A16: return vk::Format::eR16G16B16A16Unorm;
			case ColorFormat::R16_SNORM: return vk::Format::eR16Snorm;
			case ColorFormat::R16G16_SNORM: return vk::Format::eR16G16Snorm;
			case ColorFormat::R16G16B16A16_SNORM: return vk::Format::eR16G16B16A16Snorm;
			case ColorFormat::R16_UINT: return vk::Format::eR16Uint;
			case ColorFormat::R16G16_UINT: return vk::Format::eR16G16Uint;
			case ColorFormat::R16G16B16A16_UINT: return vk::Format::eR16G16B16A16Uint;
			case ColorFormat::R16_SINT: return vk::Format::eR16Sint;
			case ColorFormat::R16G16_SINT: return vk::Format::eR16G16Sint;
			case ColorFormat::R16G16B16A16_SINT: return vk::Format::eR16G16B16A16Sint;
			case ColorFormat::R32_UINT: return vk::Format::eR32Uint;
			case ColorFormat::R32G32_UINT: return vk::Format::eR32G32Uint;
			case ColorFormat::R32G32B32A32_UINT: return vk::Format::eR32G32B32A32Uint;
			case ColorFormat::R32_SINT: return vk::Format::eR32Sint;
			case ColorFormat::R32G32_SINT: return vk::Format::eR32G32Sint;
			case ColorFormat::R32G32B32A32_SINT: return vk::Format::eR32G32B32A32Sint;
			case ColorFormat::R16F: return vk::Format::eR16Sfloat;
			case ColorFormat::R16G16F: return vk::Format::eR16G16Sfloat;
			case ColorFormat::R16G16B16A16F: return vk::Format::eR16G16B16A16Sfloat;
			case ColorFormat::R32F: return vk::Format::eR32Sfloat;
			case ColorFormat::R32G32F: return vk::Format::eR32G32Sfloat;
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
			case ColorFormat::Depth: return vk::Format::eD32Sfloat;
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

	constexpr vk::ImageLayout image_layout_of(RHIAccess access)
	{
		if (access & RHIAccess::RTV)
			return vk::ImageLayout::eColorAttachmentOptimal;

		if (access & RHIAccess::DSV)
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;

		if (access & RHIAccess::UAVCompute || access & RHIAccess::UAVGraphics)
			return vk::ImageLayout::eGeneral;

		if (access & RHIAccess::CopyDst)
			return vk::ImageLayout::eTransferDstOptimal;

		if (access & RHIAccess::ResolveDst)
			return vk::ImageLayout::eColorAttachmentOptimal;

		if (access & RHIAccess::Present)
			return vk::ImageLayout::ePresentSrcKHR;

		if (access & RHIAccess::CopySrc)
			return vk::ImageLayout::eTransferSrcOptimal;

		if (access & RHIAccess::ResolveSrc)
			return vk::ImageLayout::eShaderReadOnlyOptimal;

		if (access & RHIAccess::SRVCompute || access & RHIAccess::SRVGraphics)
			return vk::ImageLayout::eShaderReadOnlyOptimal;

		if (access & RHIAccess::VertexBuffer || access & RHIAccess::IndirectArgs || access & RHIAccess::IndexBuffer)
			return vk::ImageLayout::eReadOnlyOptimalKHR;

		if (access & RHIAccess::CPURead)
			return vk::ImageLayout::eGeneral;

		if (access == RHIAccess::Undefined)
			return vk::ImageLayout::eUndefined;

		return vk::ImageLayout::eGeneral;
	}

	static inline vk::PipelineStageFlags pipeline_stage_of(RHIAccess access)
	{
		vk::PipelineStageFlags stages = vk::PipelineStageFlagBits::eNone;

		if (access & RHIAccess::CPURead)
			stages |= vk::PipelineStageFlagBits::eHost;
		if (access & RHIAccess::CPUWrite)
			stages |= vk::PipelineStageFlagBits::eHost;
		if (access & RHIAccess::IndirectArgs)
			stages |= vk::PipelineStageFlagBits::eDrawIndirect;
		if (access & RHIAccess::VertexBuffer)
			stages |= vk::PipelineStageFlagBits::eVertexInput;
		if (access & RHIAccess::IndexBuffer)
			stages |= vk::PipelineStageFlagBits::eVertexInput;
		if (access & RHIAccess::UniformBuffer)
			stages |= vk::PipelineStageFlagBits::eAllGraphics | vk::PipelineStageFlagBits::eComputeShader;
		if (access & RHIAccess::SRVCompute)
			stages |= vk::PipelineStageFlagBits::eComputeShader;
		if (access & RHIAccess::SRVGraphics)
			stages |= vk::PipelineStageFlagBits::eAllGraphics;
		if (access & RHIAccess::CopySrc)
			stages |= vk::PipelineStageFlagBits::eTransfer;
		if (access & RHIAccess::ResolveSrc)
			stages |= vk::PipelineStageFlagBits::eTransfer;
		if (access & RHIAccess::Present)
			stages |= vk::PipelineStageFlagBits::eNone;

		if (access & RHIAccess::UAVCompute)
			stages |= vk::PipelineStageFlagBits::eComputeShader;
		if (access & RHIAccess::UAVGraphics)
			stages |= vk::PipelineStageFlagBits::eAllGraphics;
		if (access & RHIAccess::CopyDst)
			stages |= vk::PipelineStageFlagBits::eTransfer;
		if (access & RHIAccess::ResolveDst)
			stages |= vk::PipelineStageFlagBits::eTransfer;
		if (access & RHIAccess::RTV)
			stages |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		if (access & RHIAccess::DSV)
			stages |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;

		return stages;
	}

	static inline vk::AccessFlags access_of(RHIAccess access)
	{
		vk::AccessFlags flags = {};

		if (access & RHIAccess::CPURead)
			flags |= vk::AccessFlagBits::eHostRead;
		if (access & RHIAccess::CPUWrite)
			flags |= vk::AccessFlagBits::eHostWrite;
		if (access & RHIAccess::IndirectArgs)
			flags |= vk::AccessFlagBits::eIndirectCommandRead;
		if (access & RHIAccess::VertexBuffer)
			flags |= vk::AccessFlagBits::eVertexAttributeRead;
		if (access & RHIAccess::IndexBuffer)
			flags |= vk::AccessFlagBits::eIndexRead;
		if (access & RHIAccess::UniformBuffer)
			flags |= vk::AccessFlagBits::eUniformRead;
		if (access & RHIAccess::SRVCompute)
			flags |= vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::SRVGraphics)
			flags |= vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::CopySrc)
			flags |= vk::AccessFlagBits::eTransferRead;
		if (access & RHIAccess::ResolveSrc)
			flags |= vk::AccessFlagBits::eTransferRead;
		if (access & RHIAccess::UAVCompute)
			flags |= vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::UAVGraphics)
			flags |= vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::CopyDst)
			flags |= vk::AccessFlagBits::eTransferWrite;
		if (access & RHIAccess::ResolveDst)
			flags |= vk::AccessFlagBits::eTransferWrite;
		if (access & RHIAccess::RTV)
			flags |= vk::AccessFlagBits::eColorAttachmentWrite;
		if (access & RHIAccess::DSV)
			flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		return flags;
	}
}// namespace Engine::VulkanEnums
