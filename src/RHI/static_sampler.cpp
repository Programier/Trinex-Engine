#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/vector.hpp>
#include <Core/threading.hpp>
#include <RHI/initializers.hpp>
#include <RHI/rhi.hpp>
#include <RHI/static_sampler.hpp>

namespace Engine
{
	static Vector<RHISampler*> s_static_samplers;

	void RHIStaticSamplerBase::create(RHISampler*& sampler, RHISamplerFilter filter, RHISamplerAddressMode address_u,
	                                  RHISamplerAddressMode address_v, RHISamplerAddressMode address_w, float mip_bias,
	                                  float max_anisotropy, Color border_color, RHICompareFunc compare_func)
	{
		RHISamplerInitializer initializer;
		initializer.filter       = filter;
		initializer.address_u    = address_u;
		initializer.address_v    = address_v;
		initializer.address_w    = address_w;
		initializer.compare_func = compare_func;
		initializer.border_color = border_color;
		initializer.anisotropy   = max_anisotropy;
		initializer.mip_lod_bias = mip_bias;

		sampler = rhi->create_sampler(&initializer);
		s_static_samplers.push_back(sampler);
	}

	static void destroy_default_samplers()
	{
		for (RHISampler* sampler : s_static_samplers)
		{
			sampler->release();
		}
	}

	static DestroyController on_destroy(destroy_default_samplers);
}// namespace Engine
