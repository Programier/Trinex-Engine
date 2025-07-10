#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/vector.hpp>
#include <Core/threading.hpp>
#include <RHI/rhi.hpp>
#include <RHI/rhi_initializers.hpp>
#include <RHI/static_sampler.hpp>

namespace Engine
{
	static Vector<RHI_Sampler*> s_static_samplers;

	void RHIStaticSamplerBase::create(RHI_Sampler*& sampler, RHISamplerFilter filter, RHISamplerAddressMode address_u,
	                                  RHISamplerAddressMode address_v, RHISamplerAddressMode address_w, float mip_bias,
	                                  float max_anisotropy, Color border_color, RHICompareFunc compare_func)
	{
		if (is_in_render_thread())
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
		else
		{
			render_thread()->call([=, &sampler]() {
				create(sampler, filter, address_u, address_v, address_w, mip_bias, max_anisotropy, border_color, compare_func);
			});
		}
	}

	static void destroy_default_samplers()
	{
		render_thread()->call([]() {
			for (RHI_Sampler* sampler : s_static_samplers)
			{
				sampler->release();
			}
		});
	}

	static DestroyController on_destroy(destroy_default_samplers);
}// namespace Engine
