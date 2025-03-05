#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	implement_engine_class(Sampler, Refl::Class::IsAsset)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, border_color);
		trinex_refl_prop(self, This, mip_lod_bias);
		trinex_refl_prop(self, This, anisotropy);
		trinex_refl_prop(self, This, min_lod);
		trinex_refl_prop(self, This, max_lod);

		trinex_refl_prop(self, This, filter);
		trinex_refl_prop(self, This, address_u)->tooltip("Address mode for U of sampler");
		trinex_refl_prop(self, This, address_v)->tooltip("Address mode for V of sampler");
		trinex_refl_prop(self, This, address_w)->tooltip("Address mode for W of sampler");
		trinex_refl_prop(self, This, compare_mode);
		trinex_refl_prop(self, This, compare_func)->tooltip("Compare Func of sampler");

		trinex_refl_prop(self, This, unnormalized_coordinates);
	}

	Sampler& Sampler::rhi_init()
	{
		m_rhi_object.reset(rhi->create_sampler(this));
		return *this;
	}

	bool Sampler::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		return archive.serialize(filter, address_u, address_v, address_w, mip_lod_bias, anisotropy, compare_mode, min_lod,
								 max_lod, compare_func, unnormalized_coordinates);
	}

	Sampler& Sampler::apply_changes()
	{
		init_resource();
		return *this;
	}
}// namespace Engine
