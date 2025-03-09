#pragma once
#include <Core/render_resource.hpp>

namespace Engine
{
	class ENGINE_EXPORT Sampler : public RenderResource
	{
		trinex_declare_class(Sampler, RenderResource);

		RenderResourcePtr<RHI_Sampler> m_sampler;

	public:
		Vector4f border_color         = {0.f, 0.f, 0.f, 1.f};
		SamplerFilter filter          = SamplerFilter::Point;
		SamplerAddressMode address_u  = SamplerAddressMode::Repeat;
		SamplerAddressMode address_v  = SamplerAddressMode::Repeat;
		SamplerAddressMode address_w  = SamplerAddressMode::Repeat;
		float mip_lod_bias            = 0.0;
		float anisotropy              = 1.0;
		CompareMode compare_mode      = CompareMode::None;
		float min_lod                 = -1000.0;
		float max_lod                 = 1000.0;
		CompareFunc compare_func      = CompareFunc::Always;
		bool unnormalized_coordinates = false;

		Sampler& init_render_resources() override;
		Sampler& release_render_resources() override;
		Sampler& rhi_bind(byte location);
		bool serialize(Archive& archive) override;
		Sampler& apply_changes() override;

		inline RHI_Sampler* rhi_sampler() const { return m_sampler; }
	};
}// namespace Engine
