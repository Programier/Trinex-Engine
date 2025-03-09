#pragma once
#include <Core/enums.hpp>
#include <d3d11.h>

namespace Engine
{
	D3D11_COMPARISON_FUNC comparison_func_of(CompareFunc func);
	D3D11_STENCIL_OP stencil_op_of(StencilOp op);
	D3D11_PRIMITIVE_TOPOLOGY primitive_topology_of(PrimitiveTopology topology);
	UINT8 component_mask_of(ColorComponent component_mask);
	D3D11_BLEND blend_func_of(BlendFunc func);
	D3D11_BLEND_OP blend_op_of(BlendOp op);
	D3D11_FILL_MODE fill_mode_of(PolygonMode mode);
	D3D11_CULL_MODE cull_mode_of(CullMode mode);
	D3D11_TEXTURE_ADDRESS_MODE address_mode_of(SamplerAddressMode value);
	D3D11_FILTER filter_of(SamplerFilter filter, bool comparison_enabled);

	DXGI_FORMAT texture_format_of(ColorFormat format);
	DXGI_FORMAT view_format_of(DXGI_FORMAT format);
	DXGI_FORMAT render_view_format_of(DXGI_FORMAT format);
}// namespace Engine
