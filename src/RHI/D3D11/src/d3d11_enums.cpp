#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <d3d11_enums.hpp>


namespace Engine
{
	D3D11_COMPARISON_FUNC comparison_func_of(CompareFunc func)
	{
		switch (func)
		{
			case CompareFunc::Always:
				return D3D11_COMPARISON_ALWAYS;
			case CompareFunc::Lequal:
				return D3D11_COMPARISON_LESS_EQUAL;
			case CompareFunc::Gequal:
				return D3D11_COMPARISON_GREATER_EQUAL;
			case CompareFunc::Less:
				return D3D11_COMPARISON_LESS;
			case CompareFunc::Greater:
				return D3D11_COMPARISON_GREATER;
			case CompareFunc::Equal:
				return D3D11_COMPARISON_EQUAL;
			case CompareFunc::NotEqual:
				return D3D11_COMPARISON_NOT_EQUAL;
			case CompareFunc::Never:
				return D3D11_COMPARISON_NEVER;
			default:
				return D3D11_COMPARISON_ALWAYS;
		}
	}

	D3D11_STENCIL_OP stencil_op_of(StencilOp op)
	{
		switch (op)
		{
			case StencilOp::Keep:
				return D3D11_STENCIL_OP_KEEP;
			case StencilOp::Zero:
				return D3D11_STENCIL_OP_ZERO;
			case StencilOp::Replace:
				return D3D11_STENCIL_OP_REPLACE;
			case StencilOp::Incr:
				return D3D11_STENCIL_OP_INCR;
			case StencilOp::IncrWrap:
				return D3D11_STENCIL_OP_INCR_SAT;
			case StencilOp::Decr:
				return D3D11_STENCIL_OP_DECR;
			case StencilOp::DecrWrap:
				return D3D11_STENCIL_OP_DECR_SAT;
			case StencilOp::Invert:
				return D3D11_STENCIL_OP_INVERT;
			default:
				return D3D11_STENCIL_OP_KEEP;
		}
	}

	D3D11_PRIMITIVE_TOPOLOGY primitive_topology_of(PrimitiveTopology topology)
	{
		switch (topology)
		{
			case PrimitiveTopology::TriangleList:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			case PrimitiveTopology::PointList:
				return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			case PrimitiveTopology::LineList:
				return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			case PrimitiveTopology::LineStrip:
				return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			case PrimitiveTopology::TriangleStrip:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			default:
				return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	UINT8 component_mask_of(ColorComponentMask component_mask)
	{
		UINT8 value     = 0;
		EnumerateType R = enum_value_of(ColorComponent::R);
		EnumerateType G = enum_value_of(ColorComponent::G);
		EnumerateType B = enum_value_of(ColorComponent::B);
		EnumerateType A = enum_value_of(ColorComponent::A);

		auto mask = enum_value_of(component_mask);

		if ((mask & R) == R)
		{
			value |= D3D11_COLOR_WRITE_ENABLE_RED;
		}
		if ((mask & G) == G)
		{
			value |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		}
		if ((mask & B) == B)
		{
			value |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		}
		if ((mask & A) == A)
		{
			value |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
		}

		return value;
	}

	D3D11_BLEND blend_func_of(BlendFunc func)
	{
		switch (func)
		{
			case BlendFunc::Zero:
				return D3D11_BLEND_ZERO;
			case BlendFunc::One:
				return D3D11_BLEND_ONE;
			case BlendFunc::SrcColor:
				return D3D11_BLEND_SRC_COLOR;
			case BlendFunc::OneMinusSrcColor:
				return D3D11_BLEND_INV_SRC_COLOR;
			case BlendFunc::DstColor:
				return D3D11_BLEND_DEST_COLOR;
			case BlendFunc::OneMinusDstColor:
				return D3D11_BLEND_INV_DEST_COLOR;
			case BlendFunc::SrcAlpha:
				return D3D11_BLEND_SRC_ALPHA;
			case BlendFunc::OneMinusSrcAlpha:
				return D3D11_BLEND_INV_SRC_ALPHA;
			case BlendFunc::DstAlpha:
				return D3D11_BLEND_DEST_ALPHA;
			case BlendFunc::OneMinusDstAlpha:
				return D3D11_BLEND_INV_DEST_ALPHA;
			case BlendFunc::BlendFactor:
				return D3D11_BLEND_BLEND_FACTOR;
			case BlendFunc::OneMinusBlendFactor:
				return D3D11_BLEND_INV_BLEND_FACTOR;
			default:
				return D3D11_BLEND_ZERO;
		}
	}

	D3D11_BLEND_OP blend_op_of(BlendOp op)
	{
		switch (op)
		{
			case BlendOp::Add:
				return D3D11_BLEND_OP_ADD;
			case BlendOp::Subtract:
				return D3D11_BLEND_OP_SUBTRACT;
			case BlendOp::ReverseSubtract:
				return D3D11_BLEND_OP_REV_SUBTRACT;
			case BlendOp::Min:
				return D3D11_BLEND_OP_MIN;
			case BlendOp::Max:
				return D3D11_BLEND_OP_MAX;
			default:
				return D3D11_BLEND_OP_ADD;
		}
	}


	D3D11_FILL_MODE fill_mode_of(PolygonMode mode)
	{
		switch (mode)
		{
			case PolygonMode::Fill:
				return D3D11_FILL_SOLID;
			case PolygonMode::Line:
				return D3D11_FILL_WIREFRAME;
			default:
				return D3D11_FILL_SOLID;
		}
	}

	D3D11_CULL_MODE cull_mode_of(CullMode mode)
	{
		switch (mode)
		{
			case CullMode::Back:
				return D3D11_CULL_BACK;
			case CullMode::Front:
				return D3D11_CULL_FRONT;
			default:
				return D3D11_CULL_NONE;
		}
	}

	D3D11_TEXTURE_ADDRESS_MODE address_mode_of(SamplerAddressMode value)
	{
		switch (value)
		{
			case SamplerAddressMode::Repeat:
				return D3D11_TEXTURE_ADDRESS_WRAP;
			case SamplerAddressMode::ClampToEdge:
				return D3D11_TEXTURE_ADDRESS_CLAMP;
			case SamplerAddressMode::ClampToBorder:
				return D3D11_TEXTURE_ADDRESS_BORDER;
			case SamplerAddressMode::MirroredRepeat:
				return D3D11_TEXTURE_ADDRESS_MIRROR;
			case SamplerAddressMode::MirrorClampToEdge:
				return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
			default:
				return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

	D3D11_FILTER filter_of(SamplerFilter filter, bool comparison_enabled)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
				return comparison_enabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			case SamplerFilter::Trilinear:
				return comparison_enabled ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT
				                          : D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			default:
				return comparison_enabled ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
	}

	/*
        switch (format)
        {
            case ColorFormat::Undefined:
                return vk::Format::eUndefined;
            case ColorFormat::FloatR:
                return vk::Format::eR32Sfloat;
            case ColorFormat::FloatRGBA:
                return vk::Format::eR32G32B32A32Sfloat;
            case ColorFormat::R8:
                return vk::Format::eR8Unorm;
            case ColorFormat::R8G8B8A8:
                return vk::Format::eR8G8B8A8Unorm;
            case ColorFormat::DepthStencil:
                return vk::Format::eD32SfloatS8Uint;
            case ColorFormat::ShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::FilteredShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::D32F:
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
*/
	DXGI_FORMAT format_of(ColorFormat format)
	{
		switch (format)
		{
			case ColorFormat::FloatR:
				return DXGI_FORMAT_R32_FLOAT;
			case ColorFormat::FloatRGBA:
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ColorFormat::R8:
				return DXGI_FORMAT_R8_UNORM;
			case ColorFormat::R8G8B8A8:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ColorFormat::DepthStencil:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			case ColorFormat::ShadowDepth:
				return DXGI_FORMAT_D32_FLOAT;
			case ColorFormat::FilteredShadowDepth:
				return DXGI_FORMAT_D32_FLOAT;
			case ColorFormat::D32F:
				return DXGI_FORMAT_D32_FLOAT;
			case ColorFormat::BC1:
				return DXGI_FORMAT_BC1_UNORM;
			case ColorFormat::BC2:
				return DXGI_FORMAT_BC2_UNORM;
			case ColorFormat::BC3:
				return DXGI_FORMAT_BC3_UNORM;
			default:
				throw EngineException("Undefined color format");
		}
	}
}// namespace Engine
