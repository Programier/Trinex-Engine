#pragma once
#include <Core/enums.hpp>
#include <Graphics/types/color_format.hpp>
#include <Graphics/types/rhi_access.hpp>
#include <dx12_headers.hpp>


namespace Engine
{
	static inline const char* semantic_name(VertexBufferSemantic semantic)
	{
		switch (semantic)
		{
			case VertexBufferSemantic::Position: return "POSITION";
			case VertexBufferSemantic::TexCoord: return "TEXCOORD";
			case VertexBufferSemantic::Color: return "COLOR";
			case VertexBufferSemantic::Normal: return "NORMAL";
			case VertexBufferSemantic::Tangent: return "TANGENT";
			case VertexBufferSemantic::Bitangent: return "BINORMAL";
			case VertexBufferSemantic::BlendWeight: return "BLENDWEIGHT";
			case VertexBufferSemantic::BlendIndices: return "BLENDINDICES";
			default: return "POSITION";
		}
	}

	static inline DXGI_FORMAT format_of(VertexBufferElementType type)
	{
		switch (type)
		{
			case VertexBufferElementType::Float1: return DXGI_FORMAT_R32_FLOAT;
			case VertexBufferElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
			case VertexBufferElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
			case VertexBufferElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case VertexBufferElementType::Byte1: return DXGI_FORMAT_R8_SINT;
			case VertexBufferElementType::Byte2: return DXGI_FORMAT_R8G8_SINT;
			case VertexBufferElementType::Byte4: return DXGI_FORMAT_R8G8B8A8_SINT;
			case VertexBufferElementType::Byte1N: return DXGI_FORMAT_R8_SNORM;
			case VertexBufferElementType::Byte2N: return DXGI_FORMAT_R8G8_SNORM;
			case VertexBufferElementType::Byte4N: return DXGI_FORMAT_R8G8B8A8_SNORM;
			case VertexBufferElementType::UByte1: return DXGI_FORMAT_R8_UINT;
			case VertexBufferElementType::UByte2: return DXGI_FORMAT_R8G8_UINT;
			case VertexBufferElementType::UByte4: return DXGI_FORMAT_R8G8B8A8_UINT;
			case VertexBufferElementType::UByte1N: return DXGI_FORMAT_R8_UNORM;
			case VertexBufferElementType::UByte2N: return DXGI_FORMAT_R8G8_UNORM;
			case VertexBufferElementType::UByte4N: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case VertexBufferElementType::Short1: return DXGI_FORMAT_R16_SINT;
			case VertexBufferElementType::Short2: return DXGI_FORMAT_R16G16_SINT;
			case VertexBufferElementType::Short4: return DXGI_FORMAT_R16G16B16A16_SINT;
			case VertexBufferElementType::Short1N: return DXGI_FORMAT_R16_SNORM;
			case VertexBufferElementType::Short2N: return DXGI_FORMAT_R16G16_SNORM;
			case VertexBufferElementType::Short4N: return DXGI_FORMAT_R16G16B16A16_SNORM;
			case VertexBufferElementType::UShort1: return DXGI_FORMAT_R16_UINT;
			case VertexBufferElementType::UShort2: return DXGI_FORMAT_R16G16_UINT;
			case VertexBufferElementType::UShort4: return DXGI_FORMAT_R16G16B16A16_UINT;
			case VertexBufferElementType::UShort1N: return DXGI_FORMAT_R16_UNORM;
			case VertexBufferElementType::UShort2N: return DXGI_FORMAT_R16G16_UNORM;
			case VertexBufferElementType::UShort4N: return DXGI_FORMAT_R16G16B16A16_UNORM;
			case VertexBufferElementType::Int1: return DXGI_FORMAT_R32_SINT;
			case VertexBufferElementType::Int2: return DXGI_FORMAT_R32G32_SINT;
			case VertexBufferElementType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
			case VertexBufferElementType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;
			case VertexBufferElementType::UInt1: return DXGI_FORMAT_R32_UINT;
			case VertexBufferElementType::UInt2: return DXGI_FORMAT_R32G32_UINT;
			case VertexBufferElementType::UInt3: return DXGI_FORMAT_R32G32B32_UINT;
			case VertexBufferElementType::UInt4: return DXGI_FORMAT_R32G32B32A32_UINT;
			default: return DXGI_FORMAT_UNKNOWN;
		}
	}

	static inline D3D12_COMPARISON_FUNC comparison_func_of(CompareFunc func)
	{
		switch (func)
		{
			case CompareFunc::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
			case CompareFunc::Lequal: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
			case CompareFunc::Gequal: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			case CompareFunc::Less: return D3D12_COMPARISON_FUNC_LESS;
			case CompareFunc::Greater: return D3D12_COMPARISON_FUNC_GREATER;
			case CompareFunc::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
			case CompareFunc::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
			case CompareFunc::Never: return D3D12_COMPARISON_FUNC_NEVER;
			default: return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	}

	static inline D3D12_STENCIL_OP stencil_op_of(StencilOp op)
	{
		switch (op)
		{
			case StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
			case StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
			case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
			case StencilOp::Incr: return D3D12_STENCIL_OP_INCR;
			case StencilOp::IncrWrap: return D3D12_STENCIL_OP_INCR_SAT;
			case StencilOp::Decr: return D3D12_STENCIL_OP_DECR;
			case StencilOp::DecrWrap: return D3D12_STENCIL_OP_DECR_SAT;
			case StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
			default: return D3D12_STENCIL_OP_KEEP;
		}
	}

	static inline D3D12_PRIMITIVE_TOPOLOGY primitive_topology_of(PrimitiveTopology topology)
	{
		switch (topology)
		{
			case PrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			default: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	static inline D3D12_PRIMITIVE_TOPOLOGY_TYPE primitive_topology_type_of(PrimitiveTopology topology)
	{
		switch (topology)
		{
			case PrimitiveTopology::TriangleList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			case PrimitiveTopology::PointList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			case PrimitiveTopology::LineList: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case PrimitiveTopology::LineStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			case PrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			default: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	static inline UINT8 component_mask_of(ColorComponent mask)
	{
		UINT8 value = 0;

		if ((mask & ColorComponent::R))
		{
			value |= D3D12_COLOR_WRITE_ENABLE_RED;
		}
		if ((mask & ColorComponent::G))
		{
			value |= D3D12_COLOR_WRITE_ENABLE_GREEN;
		}
		if ((mask & ColorComponent::B))
		{
			value |= D3D12_COLOR_WRITE_ENABLE_BLUE;
		}
		if ((mask & ColorComponent::A))
		{
			value |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
		}

		return value;
	}

	static inline D3D12_BLEND blend_func_of(BlendFunc func)
	{
		switch (func)
		{
			case BlendFunc::Zero: return D3D12_BLEND_ZERO;
			case BlendFunc::One: return D3D12_BLEND_ONE;
			case BlendFunc::SrcColor: return D3D12_BLEND_SRC_COLOR;
			case BlendFunc::OneMinusSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
			case BlendFunc::DstColor: return D3D12_BLEND_DEST_COLOR;
			case BlendFunc::OneMinusDstColor: return D3D12_BLEND_INV_DEST_COLOR;
			case BlendFunc::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
			case BlendFunc::OneMinusSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
			case BlendFunc::DstAlpha: return D3D12_BLEND_DEST_ALPHA;
			case BlendFunc::OneMinusDstAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
			case BlendFunc::BlendFactor: return D3D12_BLEND_BLEND_FACTOR;
			case BlendFunc::OneMinusBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
			default: return D3D12_BLEND_ZERO;
		}
	}

	static inline D3D12_BLEND_OP blend_op_of(BlendOp op)
	{
		switch (op)
		{
			case BlendOp::Add: return D3D12_BLEND_OP_ADD;
			case BlendOp::Subtract: return D3D12_BLEND_OP_SUBTRACT;
			case BlendOp::ReverseSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
			case BlendOp::Min: return D3D12_BLEND_OP_MIN;
			case BlendOp::Max: return D3D12_BLEND_OP_MAX;
			default: return D3D12_BLEND_OP_ADD;
		}
	}


	static inline D3D12_FILL_MODE fill_mode_of(PolygonMode mode)
	{
		switch (mode)
		{
			case PolygonMode::Fill: return D3D12_FILL_MODE_SOLID;
			case PolygonMode::Line: return D3D12_FILL_MODE_WIREFRAME;
			default: return D3D12_FILL_MODE_SOLID;
		}
	}

	static inline D3D12_CULL_MODE cull_mode_of(CullMode mode)
	{
		switch (mode)
		{
			case CullMode::Back: return D3D12_CULL_MODE_BACK;
			case CullMode::Front: return D3D12_CULL_MODE_FRONT;
			default: return D3D12_CULL_MODE_NONE;
		}
	}

	static inline D3D12_TEXTURE_ADDRESS_MODE address_mode_of(SamplerAddressMode value)
	{
		switch (value)
		{
			case SamplerAddressMode::Repeat: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			case SamplerAddressMode::ClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			case SamplerAddressMode::ClampToBorder: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			case SamplerAddressMode::MirroredRepeat: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			case SamplerAddressMode::MirrorClampToEdge: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
			default: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	static inline D3D12_FILTER filter_of(SamplerFilter filter, bool comparison_enabled)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
				return comparison_enabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			case SamplerFilter::Trilinear:
				return comparison_enabled ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT
				                          : D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			default: return comparison_enabled ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
		}
	}

	static inline D3D12_RESOURCE_STATES resource_state_of(RHIAccess access)
	{
		D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;

		if (access & RHIAccess::CPURead)
			result |= D3D12_RESOURCE_STATE_GENERIC_READ;

		if (access & RHIAccess::Present)
			result |= D3D12_RESOURCE_STATE_PRESENT;

		if (access & RHIAccess::IndirectArgs)
			result |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;

		if (access & RHIAccess::VertexBuffer)
			result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		if (access & RHIAccess::IndexBuffer)
			result |= D3D12_RESOURCE_STATE_INDEX_BUFFER;

		if (access & RHIAccess::UniformBuffer)
			result |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		if (access & RHIAccess::SRVCompute)
			result |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		if (access & RHIAccess::SRVGraphics)
			result |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		if (access & RHIAccess::CopySrc)
			result |= D3D12_RESOURCE_STATE_COPY_SOURCE;

		if (access & RHIAccess::ResolveSrc)
			result |= D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

		if (access & RHIAccess::UAVCompute || access & RHIAccess::UAVGraphics)
			result |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		if (access & RHIAccess::CopyDst)
			result |= D3D12_RESOURCE_STATE_COPY_DEST;

		if (access & RHIAccess::ResolveDst)
			result |= D3D12_RESOURCE_STATE_RESOLVE_DEST;

		if (access & RHIAccess::RTV)
			result |= D3D12_RESOURCE_STATE_RENDER_TARGET;

		if (access & RHIAccess::DSV)
			result |= D3D12_RESOURCE_STATE_DEPTH_WRITE;

		return result;
	}

	static inline DXGI_FORMAT texture_format_of(ColorFormat format)
	{
		switch (format)
		{

			case ColorFormat::Depth: return DXGI_FORMAT_R32_TYPELESS;
			case ColorFormat::DepthStencil: return DXGI_FORMAT_R24G8_TYPELESS;
			case ColorFormat::ShadowDepth: return DXGI_FORMAT_R32_TYPELESS;
			case ColorFormat::R8: return DXGI_FORMAT_R8_UNORM;
			case ColorFormat::R8G8: return DXGI_FORMAT_R8G8_UNORM;
			case ColorFormat::R8G8B8A8: return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ColorFormat::R10G108B10A2: return DXGI_FORMAT_R10G10B10A2_UNORM;
			case ColorFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
			case ColorFormat::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
			case ColorFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
			case ColorFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
			case ColorFormat::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
			case ColorFormat::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
			case ColorFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;
			case ColorFormat::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
			case ColorFormat::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
			case ColorFormat::R16: return DXGI_FORMAT_R16_UNORM;
			case ColorFormat::R16G16: return DXGI_FORMAT_R16G16_UNORM;
			case ColorFormat::R16G16B16A16: return DXGI_FORMAT_R16G16B16A16_UNORM;
			case ColorFormat::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
			case ColorFormat::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
			case ColorFormat::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
			case ColorFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
			case ColorFormat::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
			case ColorFormat::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
			case ColorFormat::R16_SINT: return DXGI_FORMAT_R16_SINT;
			case ColorFormat::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
			case ColorFormat::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
			case ColorFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
			case ColorFormat::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
			case ColorFormat::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
			case ColorFormat::R32_SINT: return DXGI_FORMAT_R32_SINT;
			case ColorFormat::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
			case ColorFormat::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
			case ColorFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
			case ColorFormat::R16G16F: return DXGI_FORMAT_R16G16_FLOAT;
			case ColorFormat::R16G16B16A16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ColorFormat::R32F: return DXGI_FORMAT_R32_FLOAT;
			case ColorFormat::R32G32F: return DXGI_FORMAT_R32G32_FLOAT;
			case ColorFormat::R32G32B32A32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ColorFormat::BC1_RGBA: return DXGI_FORMAT_BC1_UNORM;
			case ColorFormat::BC2_RGBA: return DXGI_FORMAT_BC2_UNORM;
			case ColorFormat::BC3_RGBA: return DXGI_FORMAT_BC3_UNORM;
			case ColorFormat::BC4_R: return DXGI_FORMAT_BC4_UNORM;
			case ColorFormat::BC5_RG: return DXGI_FORMAT_BC5_UNORM;
			case ColorFormat::BC7_RGBA: return DXGI_FORMAT_BC7_UNORM;
			case ColorFormat::NV12: return DXGI_FORMAT_NV12;
			case ColorFormat::P010: return DXGI_FORMAT_P010;

			default: return DXGI_FORMAT_UNKNOWN;
		}
	}

	static inline DXGI_FORMAT graphics_view_format_of(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_FLOAT;
			default: return format;
		}
	}

	static inline DXGI_FORMAT render_view_format_of(DXGI_FORMAT format)
	{
		switch (format)
		{
			case DXGI_FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case DXGI_FORMAT_R32_TYPELESS: return DXGI_FORMAT_D32_FLOAT;
			default: return format;
		}
	}
}// namespace Engine
