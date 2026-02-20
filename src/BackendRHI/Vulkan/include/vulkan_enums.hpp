#pragma once
#include <RHI/enums.hpp>
#include <vulkan_headers.hpp>


namespace Engine::VulkanEnums
{
	constexpr inline vk::ImageAspectFlags aspect_of(RHIColorFormat format)
	{
		if (format.is_color())
		{
			return vk::ImageAspectFlagBits::eColor;
		}
		else if (format.is_depth_stencil())
		{
			return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		}
		else
		{
			return vk::ImageAspectFlagBits::eDepth;
		}
	}

	constexpr inline vk::Format format_of(RHIColorFormat format)
	{
		switch (format)
		{
			case RHIColorFormat::R8: return vk::Format::eR8Unorm;
			case RHIColorFormat::R8G8: return vk::Format::eR8G8Unorm;
			case RHIColorFormat::R8G8B8A8: return vk::Format::eR8G8B8A8Unorm;
			case RHIColorFormat::R10G10B10A2: return vk::Format::eA2R10G10B10UnormPack32;
			case RHIColorFormat::R8_SNORM: return vk::Format::eR8Snorm;
			case RHIColorFormat::R8G8_SNORM: return vk::Format::eR8G8Snorm;
			case RHIColorFormat::R8G8B8A8_SNORM: return vk::Format::eR8G8B8A8Snorm;
			case RHIColorFormat::R8_UINT: return vk::Format::eR8Uint;
			case RHIColorFormat::R8G8_UINT: return vk::Format::eR8G8Uint;
			case RHIColorFormat::R8G8B8A8_UINT: return vk::Format::eR8G8B8A8Uint;
			case RHIColorFormat::R8_SINT: return vk::Format::eR8Sint;
			case RHIColorFormat::R8G8_SINT: return vk::Format::eR8G8Sint;
			case RHIColorFormat::R8G8B8A8_SINT: return vk::Format::eR8G8B8A8Sint;
			case RHIColorFormat::R16: return vk::Format::eR16Unorm;
			case RHIColorFormat::R16G16: return vk::Format::eR16G16Unorm;
			case RHIColorFormat::R16G16B16A16: return vk::Format::eR16G16B16A16Unorm;
			case RHIColorFormat::R16_SNORM: return vk::Format::eR16Snorm;
			case RHIColorFormat::R16G16_SNORM: return vk::Format::eR16G16Snorm;
			case RHIColorFormat::R16G16B16A16_SNORM: return vk::Format::eR16G16B16A16Snorm;
			case RHIColorFormat::R16_UINT: return vk::Format::eR16Uint;
			case RHIColorFormat::R16G16_UINT: return vk::Format::eR16G16Uint;
			case RHIColorFormat::R16G16B16A16_UINT: return vk::Format::eR16G16B16A16Uint;
			case RHIColorFormat::R16_SINT: return vk::Format::eR16Sint;
			case RHIColorFormat::R16G16_SINT: return vk::Format::eR16G16Sint;
			case RHIColorFormat::R16G16B16A16_SINT: return vk::Format::eR16G16B16A16Sint;
			case RHIColorFormat::R32_UINT: return vk::Format::eR32Uint;
			case RHIColorFormat::R32G32_UINT: return vk::Format::eR32G32Uint;
			case RHIColorFormat::R32G32B32A32_UINT: return vk::Format::eR32G32B32A32Uint;
			case RHIColorFormat::R32_SINT: return vk::Format::eR32Sint;
			case RHIColorFormat::R32G32_SINT: return vk::Format::eR32G32Sint;
			case RHIColorFormat::R32G32B32A32_SINT: return vk::Format::eR32G32B32A32Sint;
			case RHIColorFormat::R16F: return vk::Format::eR16Sfloat;
			case RHIColorFormat::R16G16F: return vk::Format::eR16G16Sfloat;
			case RHIColorFormat::R16G16B16A16F: return vk::Format::eR16G16B16A16Sfloat;
			case RHIColorFormat::R32F: return vk::Format::eR32Sfloat;
			case RHIColorFormat::R32G32F: return vk::Format::eR32G32Sfloat;
			case RHIColorFormat::R32G32B32A32F: return vk::Format::eR32G32B32A32Sfloat;
			case RHIColorFormat::BC1_RGBA: return vk::Format::eBc1RgbaUnormBlock;
			case RHIColorFormat::BC2_RGBA: return vk::Format::eBc2UnormBlock;
			case RHIColorFormat::BC3_RGBA: return vk::Format::eBc3UnormBlock;
			case RHIColorFormat::BC4_R: return vk::Format::eBc4UnormBlock;
			case RHIColorFormat::BC5_RG: return vk::Format::eBc5UnormBlock;
			case RHIColorFormat::BC7_RGBA: return vk::Format::eBc7UnormBlock;
			case RHIColorFormat::ASTC_4x4_RGBA: return vk::Format::eAstc4x4UnormBlock;
			case RHIColorFormat::ASTC_6x6_RGBA: return vk::Format::eAstc6x6UnormBlock;
			case RHIColorFormat::ASTC_8x8_RGBA: return vk::Format::eAstc8x8UnormBlock;
			case RHIColorFormat::ASTC_10x10_RGBA: return vk::Format::eAstc10x10UnormBlock;
			case RHIColorFormat::ETC1_RGB: return vk::Format::eEtc2R8G8B8UnormBlock;
			case RHIColorFormat::ETC2_RGB: return vk::Format::eEtc2R8G8B8UnormBlock;
			case RHIColorFormat::ETC2_RGBA: return vk::Format::eEtc2R8G8B8A8UnormBlock;
			case RHIColorFormat::NV12: return vk::Format::eG8B8R82Plane420Unorm;
			case RHIColorFormat::P010: return vk::Format::eG10X6B10X6R10X62Plane420Unorm3Pack16;
			case RHIColorFormat::D32F: return vk::Format::eD32Sfloat;
			case RHIColorFormat::D24S8: return vk::Format::eD24UnormS8Uint;
			case RHIColorFormat::D16_UNORM: return vk::Format::eD16Unorm;

			default: return vk::Format::eUndefined;
		}
	}

	constexpr inline bool is_depth_format(vk::Format format)
	{
		switch (format)
		{
			case vk::Format::eD16Unorm:
			case vk::Format::eD16UnormS8Uint:
			case vk::Format::eX8D24UnormPack32:
			case vk::Format::eD24UnormS8Uint:
			case vk::Format::eD32Sfloat:
			case vk::Format::eD32SfloatS8Uint: return true;
			default: return false;
		}
	}

	constexpr inline bool is_stencil_format(vk::Format format)
	{
		switch (format)
		{
			case vk::Format::eS8Uint:
			case vk::Format::eD16UnormS8Uint:
			case vk::Format::eD24UnormS8Uint:
			case vk::Format::eD32SfloatS8Uint: return true;
			default: return false;
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

	constexpr inline vk::Filter filter_of(RHISamplerFilter filter)
	{
		switch (filter)
		{
			case RHISamplerFilter::Bilinear:
			case RHISamplerFilter::Trilinear: return vk::Filter::eLinear;
			default: return vk::Filter::eNearest;
		}
	}

	constexpr inline vk::BlendFactor blend_func_of(RHIBlendFunc func, bool is_for_alpha)
	{
		switch (func)
		{
			case RHIBlendFunc::Zero: return vk::BlendFactor::eZero;
			case RHIBlendFunc::One: return vk::BlendFactor::eOne;
			case RHIBlendFunc::SrcColor: return vk::BlendFactor::eSrcColor;
			case RHIBlendFunc::OneMinusSrcColor: return vk::BlendFactor::eOneMinusSrcColor;
			case RHIBlendFunc::DstColor: return vk::BlendFactor::eDstColor;
			case RHIBlendFunc::OneMinusDstColor: return vk::BlendFactor::eOneMinusDstColor;
			case RHIBlendFunc::SrcAlpha: return vk::BlendFactor::eSrcAlpha;
			case RHIBlendFunc::OneMinusSrcAlpha: return vk::BlendFactor::eOneMinusSrcAlpha;
			case RHIBlendFunc::DstAlpha: return vk::BlendFactor::eDstAlpha;
			case RHIBlendFunc::OneMinusDstAlpha: return vk::BlendFactor::eOneMinusDstAlpha;
			case RHIBlendFunc::BlendFactor:
				return is_for_alpha ? vk::BlendFactor::eConstantAlpha : vk::BlendFactor::eConstantColor;
			case RHIBlendFunc::OneMinusBlendFactor:
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

		if (access & RHIAccess::TransferDst)
			return vk::ImageLayout::eTransferDstOptimal;

		if (access & RHIAccess::ResolveDst)
			return vk::ImageLayout::eColorAttachmentOptimal;

		if (access & RHIAccess::PresentSrc)
			return vk::ImageLayout::ePresentSrcKHR;

		if (access & RHIAccess::TransferSrc)
			return vk::ImageLayout::eTransferSrcOptimal;

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
		if (access == RHIAccess::Undefined)
			return vk::PipelineStageFlagBits::eTopOfPipe;

		vk::PipelineStageFlags stages = vk::PipelineStageFlagBits::eNone;

		if (access & RHIAccess::CPURead)
			stages |= vk::PipelineStageFlagBits::eHost;
		if (access & RHIAccess::CPUWrite)
			stages |= vk::PipelineStageFlagBits::eHost;
		if (access & RHIAccess::AccelerationRead)
			stages |= vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR | vk::PipelineStageFlagBits::eRayTracingShaderKHR;
		if (access & RHIAccess::AccelerationWrite)
			stages |= vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR | vk::PipelineStageFlagBits::eRayTracingShaderKHR;
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
		if (access & RHIAccess::TransferSrc)
			stages |= vk::PipelineStageFlagBits::eTransfer;
		if (access & RHIAccess::PresentSrc)
			stages |= vk::PipelineStageFlagBits::eBottomOfPipe;

		if (access & RHIAccess::UAVCompute)
			stages |= vk::PipelineStageFlagBits::eComputeShader;
		if (access & RHIAccess::UAVGraphics)
			stages |= vk::PipelineStageFlagBits::eAllGraphics;
		if (access & RHIAccess::TransferDst)
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
		if (access & RHIAccess::AccelerationRead)
			flags |= vk::AccessFlagBits::eAccelerationStructureReadKHR | vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::AccelerationWrite)
			flags |= vk::AccessFlagBits::eAccelerationStructureWriteKHR | vk::AccessFlagBits::eShaderWrite;
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
		if (access & RHIAccess::TransferSrc)
			flags |= vk::AccessFlagBits::eTransferRead;
		if (access & RHIAccess::UAVCompute)
			flags |= vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::UAVGraphics)
			flags |= vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
		if (access & RHIAccess::TransferDst)
			flags |= vk::AccessFlagBits::eTransferWrite;
		if (access & RHIAccess::ResolveDst)
			flags |= vk::AccessFlagBits::eTransferWrite;
		if (access & RHIAccess::RTVRead)
			flags |= vk::AccessFlagBits::eColorAttachmentRead;
		if (access & RHIAccess::RTVWrite)
			flags |= vk::AccessFlagBits::eColorAttachmentWrite;
		if (access & RHIAccess::DSVRead)
			flags |= vk::AccessFlagBits::eDepthStencilAttachmentRead;
		if (access & RHIAccess::DSVWrite)
			flags |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		return flags;
	}

	static inline vk::SamplerAddressMode address_mode_of(RHISamplerAddressMode mode)
	{
		switch (mode)
		{
			case RHISamplerAddressMode::Repeat: return vk::SamplerAddressMode::eRepeat;
			case RHISamplerAddressMode::ClampToEdge: return vk::SamplerAddressMode::eClampToEdge;
			case RHISamplerAddressMode::ClampToBorder: return vk::SamplerAddressMode::eClampToBorder;
			case RHISamplerAddressMode::MirroredRepeat: return vk::SamplerAddressMode::eMirroredRepeat;
			case RHISamplerAddressMode::MirrorClampToEdge: return vk::SamplerAddressMode::eMirrorClampToEdge;
			default: return vk::SamplerAddressMode::eRepeat;
		}
	}

	static inline vk::CompareOp compare_of(RHICompareFunc func)
	{
		switch (func)
		{
			case RHICompareFunc::Always: return vk::CompareOp::eAlways;
			case RHICompareFunc::Lequal: return vk::CompareOp::eLessOrEqual;
			case RHICompareFunc::Gequal: return vk::CompareOp::eGreaterOrEqual;
			case RHICompareFunc::Less: return vk::CompareOp::eLess;
			case RHICompareFunc::Greater: return vk::CompareOp::eGreater;
			case RHICompareFunc::Equal: return vk::CompareOp::eEqual;
			case RHICompareFunc::NotEqual: return vk::CompareOp::eNotEqual;
			case RHICompareFunc::Never: return vk::CompareOp::eNever;
			default: return vk::CompareOp::eNever;
		}
	}


	static inline vk::StencilOp stencil_of(RHIStencilOp op)
	{
		switch (op)
		{
			case RHIStencilOp::Keep: return vk::StencilOp::eKeep;
			case RHIStencilOp::Zero: return vk::StencilOp::eZero;
			case RHIStencilOp::Replace: return vk::StencilOp::eReplace;
			case RHIStencilOp::Incr: return vk::StencilOp::eIncrementAndClamp;
			case RHIStencilOp::IncrWrap: return vk::StencilOp::eIncrementAndWrap;
			case RHIStencilOp::Decr: return vk::StencilOp::eDecrementAndClamp;
			case RHIStencilOp::DecrWrap: return vk::StencilOp::eDecrementAndWrap;
			case RHIStencilOp::Invert: return vk::StencilOp::eInvert;
			default: return vk::StencilOp::eKeep;
		}
	}

	static inline vk::BlendOp blend_of(RHIBlendOp op)
	{
		switch (op)
		{
			case RHIBlendOp::Add: return vk::BlendOp::eAdd;
			case RHIBlendOp::Subtract: return vk::BlendOp::eSubtract;
			case RHIBlendOp::ReverseSubtract: return vk::BlendOp::eReverseSubtract;
			case RHIBlendOp::Min: return vk::BlendOp::eMin;
			case RHIBlendOp::Max: return vk::BlendOp::eMax;
			default: return vk::BlendOp::eAdd;
		}
	}

	static inline vk::DescriptorType descriptor_type_of(RHIShaderParameterType type)
	{
		constexpr RHIShaderParameterType combined_image_sampler(RHIShaderParameterType::META_Texture |
		                                                        RHIShaderParameterType::META_Sampler);

		if ((type & combined_image_sampler) == combined_image_sampler)
			return vk::DescriptorType::eCombinedImageSampler;

		if ((type & RHIShaderParameterType::META_Texture) == RHIShaderParameterType::META_Texture)
		{
			if ((type & RHIShaderParameterType::META_RW) == RHIShaderParameterType::META_RW)
				return vk::DescriptorType::eStorageImage;
			return vk::DescriptorType::eSampledImage;
		}

		if ((type & RHIShaderParameterType::META_Sampler) == RHIShaderParameterType::META_Sampler)
			return vk::DescriptorType::eSampler;

		if ((type & RHIShaderParameterType::META_UniformBuffer) == RHIShaderParameterType::META_UniformBuffer)
			return vk::DescriptorType::eUniformBufferDynamic;

		if ((type & RHIShaderParameterType::META_Buffer) == RHIShaderParameterType::META_Buffer)
		{
			if ((type & RHIShaderParameterType::META_RW) == RHIShaderParameterType::META_RW)
				return vk::DescriptorType::eStorageTexelBuffer;
			return vk::DescriptorType::eUniformTexelBuffer;
		}

		if ((type & RHIShaderParameterType::META_StructuredBuffer) == RHIShaderParameterType::META_StructuredBuffer)
			return vk::DescriptorType::eStorageBuffer;

		if ((type & RHIShaderParameterType::META_ByteAddressBuffer) == RHIShaderParameterType::META_ByteAddressBuffer)
			return vk::DescriptorType::eStorageBuffer;

		if ((type & RHIShaderParameterType::META_AccelerationStructure) == RHIShaderParameterType::META_AccelerationStructure)
			return vk::DescriptorType::eAccelerationStructureKHR;

		trinex_unreachable_msg("Undefined descriptor type");
		return vk::DescriptorType::eSampler;
	}

	static inline vk::FrontFace face_of(RHIFrontFace face)
	{
		if (face == RHIFrontFace::CounterClockWise)
			return vk::FrontFace::eCounterClockwise;
		return vk::FrontFace::eClockwise;
	}

	static inline vk::CullModeFlags cull_mode_of(RHICullMode mode)
	{
		switch (mode)
		{
			case RHICullMode::Back: return vk::CullModeFlagBits::eBack;
			case RHICullMode::Front: return vk::CullModeFlagBits::eFront;
			default: return vk::CullModeFlagBits::eNone;
		}
	}

	static inline vk::PrimitiveTopology primitive_topology_of(RHIPrimitiveTopology topology)
	{
		switch (topology)
		{
			case RHIPrimitiveTopology::TriangleList: return vk::PrimitiveTopology::eTriangleList;
			case RHIPrimitiveTopology::TriangleStrip: return vk::PrimitiveTopology::eTriangleStrip;
			case RHIPrimitiveTopology::LineList: return vk::PrimitiveTopology::eLineList;
			case RHIPrimitiveTopology::LineStrip: return vk::PrimitiveTopology::eLineStrip;
			case RHIPrimitiveTopology::PointList: return vk::PrimitiveTopology::ePointList;
			default: return vk::PrimitiveTopology::eTriangleList;
		}
	}

	static inline vk::PolygonMode polygon_mode_of(RHIPolygonMode mode)
	{
		switch (mode)
		{
			case RHIPolygonMode::Line: return vk::PolygonMode::eLine;
			case RHIPolygonMode::Point: return vk::PolygonMode::ePoint;
			default: return vk::PolygonMode::eFill;
		}
	}

	static inline vk::ImageViewType image_view_type_of(RHITextureType type)
	{
		switch (type)
		{
			case RHITextureType::Texture1DArray: return vk::ImageViewType::e1DArray;
			case RHITextureType::Texture2D: return vk::ImageViewType::e2D;
			case RHITextureType::Texture2DArray: return vk::ImageViewType::e2DArray;
			case RHITextureType::TextureCube: return vk::ImageViewType::eCube;
			case RHITextureType::TextureCubeArray: return vk::ImageViewType::eCubeArray;
			case RHITextureType::Texture3D: return vk::ImageViewType::e3D;
			default: return vk::ImageViewType::e1D;
		}
	}

	static inline vk::Format vertex_format_of(RHIVertexFormat format)
	{
		switch (format)
		{
			// Unsigned normalized formats
			case RHIVertexFormat::R8: return vk::Format::eR8Unorm;
			case RHIVertexFormat::RG8: return vk::Format::eR8G8Unorm;
			case RHIVertexFormat::RGB8: return vk::Format::eR8G8B8Unorm;
			case RHIVertexFormat::RGBA8: return vk::Format::eR8G8B8A8Unorm;
			case RHIVertexFormat::RGB10A2: return vk::Format::eA2R10G10B10UnormPack32;
			case RHIVertexFormat::R16: return vk::Format::eR16Unorm;
			case RHIVertexFormat::RG16: return vk::Format::eR16G16Unorm;
			case RHIVertexFormat::RGB16: return vk::Format::eR16G16B16Unorm;
			case RHIVertexFormat::RGBA16: return vk::Format::eR16G16B16A16Unorm;

			// Signed normalized formats
			case RHIVertexFormat::R8S: return vk::Format::eR8Snorm;
			case RHIVertexFormat::RG8S: return vk::Format::eR8G8Snorm;
			case RHIVertexFormat::RGB8S: return vk::Format::eR8G8B8Snorm;
			case RHIVertexFormat::RGBA8S: return vk::Format::eR8G8B8A8Snorm;
			case RHIVertexFormat::RGB10A2S: return vk::Format::eA2R10G10B10SnormPack32;
			case RHIVertexFormat::R16S: return vk::Format::eR16Snorm;
			case RHIVertexFormat::RG16S: return vk::Format::eR16G16Snorm;
			case RHIVertexFormat::RGB16S: return vk::Format::eR16G16B16Snorm;
			case RHIVertexFormat::RGBA16S: return vk::Format::eR16G16B16A16Snorm;

			// Unsigned integer formats
			case RHIVertexFormat::R8UI: return vk::Format::eR8Uint;
			case RHIVertexFormat::RG8UI: return vk::Format::eR8G8Uint;
			case RHIVertexFormat::RGB8UI: return vk::Format::eR8G8B8Uint;
			case RHIVertexFormat::RGBA8UI: return vk::Format::eR8G8B8A8Uint;
			case RHIVertexFormat::RGB10A2UI: return vk::Format::eA2R10G10B10UintPack32;
			case RHIVertexFormat::R16UI: return vk::Format::eR16Uint;
			case RHIVertexFormat::RG16UI: return vk::Format::eR16G16Uint;
			case RHIVertexFormat::RGB16UI: return vk::Format::eR16G16B16Uint;
			case RHIVertexFormat::RGBA16UI: return vk::Format::eR16G16B16A16Uint;
			case RHIVertexFormat::R32UI: return vk::Format::eR32Uint;
			case RHIVertexFormat::RG32UI: return vk::Format::eR32G32Uint;
			case RHIVertexFormat::RGB32UI: return vk::Format::eR32G32B32Uint;
			case RHIVertexFormat::RGBA32UI: return vk::Format::eR32G32B32A32Uint;

			// Signed integer formats
			case RHIVertexFormat::R8SI: return vk::Format::eR8Sint;
			case RHIVertexFormat::RG8SI: return vk::Format::eR8G8Sint;
			case RHIVertexFormat::RGB8SI: return vk::Format::eR8G8B8Sint;
			case RHIVertexFormat::RGBA8SI: return vk::Format::eR8G8B8A8Sint;
			case RHIVertexFormat::RGB10A2SI: return vk::Format::eA2R10G10B10SintPack32;
			case RHIVertexFormat::R16SI: return vk::Format::eR16Sint;
			case RHIVertexFormat::RG16SI: return vk::Format::eR16G16Sint;
			case RHIVertexFormat::RGB16SI: return vk::Format::eR16G16B16Sint;
			case RHIVertexFormat::RGBA16SI: return vk::Format::eR16G16B16A16Sint;
			case RHIVertexFormat::R32SI: return vk::Format::eR32Sint;
			case RHIVertexFormat::RG32SI: return vk::Format::eR32G32Sint;
			case RHIVertexFormat::RGB32SI: return vk::Format::eR32G32B32Sint;
			case RHIVertexFormat::RGBA32SI: return vk::Format::eR32G32B32A32Sint;

			// Floating formats
			case RHIVertexFormat::R16F: return vk::Format::eR16Sfloat;
			case RHIVertexFormat::RG16F: return vk::Format::eR16G16Sfloat;
			case RHIVertexFormat::RGB16F: return vk::Format::eR16G16B16Sfloat;
			case RHIVertexFormat::RGBA16F: return vk::Format::eR16G16B16A16Sfloat;
			case RHIVertexFormat::R32F: return vk::Format::eR32Sfloat;
			case RHIVertexFormat::RG32F: return vk::Format::eR32G32Sfloat;
			case RHIVertexFormat::RGB32F: return vk::Format::eR32G32B32Sfloat;
			case RHIVertexFormat::RGBA32F: return vk::Format::eR32G32B32A32Sfloat;

			default: return vk::Format::eUndefined;
		}
	}

	static inline vk::VertexInputRate input_rate_of(RHIVertexInputRate rate)
	{
		return rate == RHIVertexInputRate::Vertex ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance;
	}

	static inline vk::ColorComponentFlags color_component_flags_of(RHIColorComponent mask)
	{
		vk::ColorComponentFlags color_mask;

		if (mask & RHIColorComponent::R)
		{
			color_mask |= vk::ColorComponentFlagBits::eR;
		}

		if (mask & RHIColorComponent::G)
		{
			color_mask |= vk::ColorComponentFlagBits::eG;
		}

		if (mask & RHIColorComponent::B)
		{
			color_mask |= vk::ColorComponentFlagBits::eB;
		}

		if (mask & RHIColorComponent::A)
		{
			color_mask |= vk::ColorComponentFlagBits::eA;
		}
		return color_mask;
	}

	static inline vk::FragmentShadingRateCombinerOpKHR shading_rate_combiner_of(RHIShadingRateCombiner combiner)
	{
		switch (combiner)
		{
			case RHIShadingRateCombiner::Keep: return vk::FragmentShadingRateCombinerOpKHR::eKeep;
			case RHIShadingRateCombiner::Replace: return vk::FragmentShadingRateCombinerOpKHR::eReplace;
			case RHIShadingRateCombiner::Min: return vk::FragmentShadingRateCombinerOpKHR::eMin;
			case RHIShadingRateCombiner::Max: return vk::FragmentShadingRateCombinerOpKHR::eMax;
			case RHIShadingRateCombiner::Mul: return vk::FragmentShadingRateCombinerOpKHR::eMul;
			default: return vk::FragmentShadingRateCombinerOpKHR::eKeep;
		}
	}

	static constexpr inline vk::AttachmentLoadOp load_of(RHILoadFunc func)
	{
		switch (func)
		{
			case RHILoadFunc::Clear: return vk::AttachmentLoadOp::eClear;
			case RHILoadFunc::DontCare: return vk::AttachmentLoadOp::eDontCare;
			default: return vk::AttachmentLoadOp::eLoad;
		}
	}

	static constexpr inline vk::AttachmentStoreOp store_of(RHIStoreFunc func)
	{
		switch (func)
		{
			case RHIStoreFunc::DontCare: return vk::AttachmentStoreOp::eDontCare;
			default: return vk::AttachmentStoreOp::eStore;
		}
	}

	static constexpr inline vk::ResolveModeFlagBits resolve_mode_of(RHIResolveFunc func)
	{
		switch (func)
		{
			case RHIResolveFunc::Sample0: return vk::ResolveModeFlagBits::eSampleZero;
			case RHIResolveFunc::Average: return vk::ResolveModeFlagBits::eAverage;
			case RHIResolveFunc::Min: return vk::ResolveModeFlagBits::eMin;
			case RHIResolveFunc::Max: return vk::ResolveModeFlagBits::eMax;
			default: return vk::ResolveModeFlagBits::eNone;
		}
	}

	static constexpr inline vk::RenderingFlags rendering_flags_of(RHIRenderingFlags flags)
	{
		vk::RenderingFlags result = {};

		if (flags & RHIRenderingFlags::Suspending)
			result |= vk::RenderingFlagBits::eSuspending;

		if (flags & RHIRenderingFlags::Resuming)
			result |= vk::RenderingFlagBits::eResuming;

		if (flags & RHIRenderingFlags::SecondaryBuffersOnly)
			result |= vk::RenderingFlagBits::eContentsSecondaryCommandBuffers;

		return result;
	}

	static constexpr inline vk::SampleCountFlagBits sample_count_of(RHITextureCreateFlags flags)
	{
		if (flags & RHITextureCreateFlags::Samples8)
			return vk::SampleCountFlagBits::e8;

		if (flags & RHITextureCreateFlags::Samples4)
			return vk::SampleCountFlagBits::e4;

		if (flags & RHITextureCreateFlags::Samples2)
			return vk::SampleCountFlagBits::e2;

		return vk::SampleCountFlagBits::e1;
	}

	static constexpr inline vk::SampleCountFlagBits sample_count_of(RHISampleCount count)
	{
		switch (count)
		{
			case RHISampleCount::x2: return vk::SampleCountFlagBits::e2;
			case RHISampleCount::x4: return vk::SampleCountFlagBits::e4;
			case RHISampleCount::x8: return vk::SampleCountFlagBits::e8;
			default: return vk::SampleCountFlagBits::e1;
		}
	}
}// namespace Engine::VulkanEnums
