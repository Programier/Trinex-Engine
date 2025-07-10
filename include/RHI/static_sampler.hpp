#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	struct RHI_Sampler;

	class RHIStaticSamplerBase
	{
	protected:
		static ENGINE_EXPORT void create(RHI_Sampler*& sampler, RHISamplerFilter filter, RHISamplerAddressMode address_u,
		                                 RHISamplerAddressMode address_v, RHISamplerAddressMode address_w, float mip_bias,
		                                 float max_anisotropy, Color border_color, RHICompareFunc compare_func);
	};

	template<RHISamplerFilter filter         = RHISamplerFilter::Point,           //
	         RHISamplerAddressMode address_u = RHISamplerAddressMode::ClampToEdge,//
	         RHISamplerAddressMode address_v = RHISamplerAddressMode::ClampToEdge,//
	         RHISamplerAddressMode address_w = RHISamplerAddressMode::ClampToEdge,//
	         float mip_bias                  = 0.f,                               //
	         float max_anisotropy            = 1.f,                               //
	         Color border_color              = Color(0, 0, 0, 0),                 //
	         RHICompareFunc compare_func     = RHICompareFunc::Never>
	class RHIStaticSampler final : public RHIStaticSamplerBase
	{
	private:
		struct StaticResource {
			RHI_Sampler* sampler = nullptr;
			StaticResource()
			{
				RHIStaticSamplerBase::create(sampler, filter, address_u, address_v, address_w, mip_bias, max_anisotropy,
				                             border_color, compare_func);
			}
		};

	public:
		static inline RHI_Sampler* static_sampler()
		{
			static StaticResource s_resource;
			return s_resource.sampler;
		}
	};

	using RHIPointSampler     = RHIStaticSampler<RHISamplerFilter::Point>;
	using RHIBilinearSampler  = RHIStaticSampler<RHISamplerFilter::Bilinear>;
	using RHITrilinearSampler = RHIStaticSampler<RHISamplerFilter::Trilinear>;

	using RHIPointWrapSampler     = RHIStaticSampler<RHISamplerFilter::Point, RHISamplerAddressMode::Repeat,
	                                                 RHISamplerAddressMode::Repeat, RHISamplerAddressMode::Repeat>;
	using RHIBilinearWrapSampler  = RHIStaticSampler<RHISamplerFilter::Bilinear, RHISamplerAddressMode::Repeat,
	                                                 RHISamplerAddressMode::Repeat, RHISamplerAddressMode::Repeat>;
	using RHITrilinearWrapSampler = RHIStaticSampler<RHISamplerFilter::Trilinear, RHISamplerAddressMode::Repeat,
	                                                 RHISamplerAddressMode::Repeat, RHISamplerAddressMode::Repeat>;

	using RHIShadowSampler = RHIStaticSampler<RHISamplerFilter::Bilinear, RHISamplerAddressMode::ClampToBorder,
	                                          RHISamplerAddressMode::ClampToBorder, RHISamplerAddressMode::ClampToBorder, 0.0f,
	                                          1.0f, Color(0, 0, 0, 0), RHICompareFunc::Lequal>;
}// namespace Engine
