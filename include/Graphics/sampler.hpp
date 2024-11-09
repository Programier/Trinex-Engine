#pragma once
#include <Core/render_resource.hpp>

namespace Engine
{


	class ENGINE_EXPORT Sampler : public BindedRenderResource
	{
		declare_class(Sampler, BindedRenderResource);

	public:
		Vector4D border_color         = {0.f, 0.f, 0.f, 1.f};
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

		Sampler& rhi_create() override;
		bool serialize(Archive& archive) override;
		Sampler& apply_changes() override;
	};
}// namespace Engine
