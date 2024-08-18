#include <Core/logger.hpp>
#include <Graphics/sampler.hpp>
#include <d3d11_api.hpp>
#include <d3d11_enums.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_sampler.hpp>

namespace Engine
{
	static FORCE_INLINE bool is_comparison_mode(CompareFunc func)
	{
		return func != CompareFunc::Never && func != CompareFunc::Always;
	}

	bool D3D11_Sampler::init(const Sampler* sampler)
	{
		D3D11_SAMPLER_DESC desc{};

		desc.Filter			= filter_of(sampler->filter, is_comparison_mode(sampler->compare_func));
		desc.AddressU		= address_mode_of(sampler->address_u);
		desc.AddressV		= address_mode_of(sampler->address_v);
		desc.AddressW		= address_mode_of(sampler->address_w);
		desc.MipLODBias		= sampler->mip_lod_bias;
		desc.MaxAnisotropy	= sampler->anisotropy;
		desc.ComparisonFunc = comparison_func_of(sampler->compare_func);
		desc.BorderColor[0] = sampler->border_color.r;
		desc.BorderColor[1] = sampler->border_color.g;
		desc.BorderColor[2] = sampler->border_color.b;
		desc.BorderColor[3] = sampler->border_color.a;
		desc.MinLOD			= sampler->min_lod;
		desc.MaxLOD			= sampler->max_lod;

		return DXAPI->m_device->CreateSamplerState(&desc, &m_sampler) == S_OK;
	}

	void D3D11_Sampler::bind(BindLocation location)
	{
		auto& pipeline = DXAPI->m_state.pipeline;

		if (pipeline == nullptr)
			return;

		if (pipeline->m_vertex_shader)
			DXAPI->m_context->VSSetSamplers(location.binding, 1, &m_sampler);
		if (pipeline->m_tsc_shader)
			DXAPI->m_context->HSSetSamplers(location.binding, 1, &m_sampler);
		if (pipeline->m_ts_shader)
			DXAPI->m_context->DSSetSamplers(location.binding, 1, &m_sampler);
		if (pipeline->m_geometry_shader)
			DXAPI->m_context->GSSetSamplers(location.binding, 1, &m_sampler);
		if (pipeline->m_fragment_shader)
			DXAPI->m_context->PSSetSamplers(location.binding, 1, &m_sampler);
	}

	D3D11_Sampler::~D3D11_Sampler()
	{
		d3d11_release(m_sampler);
	}

	RHI_Sampler* D3D11::create_sampler(const Sampler* sampler)
	{
		D3D11_Sampler* d3d11_sampler = new D3D11_Sampler();
		if (!d3d11_sampler->init(sampler))
		{
			error_log("D3D11 Sampler", "Failed to create sampler!");
			delete d3d11_sampler;
			d3d11_sampler = nullptr;
		}
		return d3d11_sampler;
	}
}// namespace Engine
