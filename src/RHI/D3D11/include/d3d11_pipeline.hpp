#pragma once
#include <Graphics/rhi.hpp>
#include <d3d11.h>

namespace Engine
{
	class D3D11_VertexShader;
	class D3D11_TesselationControlShader;
	class D3D11_TesselationShader;
	class D3D11_GeometryShader;
	class D3D11_FragmentShader;

	class D3D11_Pipeline : public RHI_DefaultDestroyable<RHI_Pipeline>
	{
	public:
		const Pipeline* m_engine_pipeline = nullptr;

		D3D11_VertexShader* m_vertex_shader			 = nullptr;
		D3D11_TesselationControlShader* m_tsc_shader = nullptr;
		D3D11_TesselationShader* m_ts_shader		 = nullptr;
		D3D11_GeometryShader* m_geometry_shader		 = nullptr;
		D3D11_FragmentShader* m_fragment_shader		 = nullptr;

		D3D11_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		ID3D11DepthStencilState* m_depth_stencil_state = nullptr;
		ID3D11BlendState* m_blend_state				   = nullptr;
		ID3D11RasterizerState* m_rasterizer_state	   = nullptr;


		bool init(const class Pipeline* pipeline);
		void bind() override;
		static void unbind();
		~D3D11_Pipeline();
	};
}// namespace Engine
