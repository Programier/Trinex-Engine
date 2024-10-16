#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/reflection/enum.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>

namespace Engine
{
	implement_engine_class(Sampler, Class::IsAsset)
	{
		Class* self                   = static_class_instance();
		Refl::Enum* address_mode_enum = Refl::Enum::static_find("Engine::SamplerAddressMode", Refl::FindFlags::IsRequired);

		self->add_properties(new ClassProperty("Border color", "Border color", &This::border_color),

							 new ClassProperty("Mip lod bias", "Mip lod bias of sampler", &This::mip_lod_bias),
							 new ClassProperty("Anisotropy", "Anisotropy of sampler", &This::anisotropy),
							 new ClassProperty("Min lod", "Min lod of sampler", &This::min_lod),
							 new ClassProperty("Max lod", "Max lod of sampler", &This::max_lod),
							 new EnumProperty("Filter", "Filter of sampler", &This::filter,
											  Refl::Enum::static_find("Engine::SamplerFilter", Refl::FindFlags::IsRequired)),
							 new EnumProperty("Address U", "Address mode for U of sampler", &This::address_u, address_mode_enum),
							 new EnumProperty("Address V", "Address mode for V of sampler", &This::address_v, address_mode_enum),
							 new EnumProperty("Address W", "Address mode for W of sampler", &This::address_w, address_mode_enum),
							 new EnumProperty("Compare Mode", "Compare Mode of sampler", &This::compare_mode,
											  Refl::Enum::static_find("Engine::CompareMode", Refl::FindFlags::IsRequired)),
							 new EnumProperty("Compare Func", "Compare Func of sampler", &This::compare_func,
											  Refl::Enum::static_find("Engine::CompareFunc", Refl::FindFlags::IsRequired)),
							 new ClassProperty("Unnormalized Coordinates", "True if sampler used unnormalized coordinates",
											   &This::unnormalized_coordinates));
	}

	Sampler& Sampler::rhi_create()
	{
		m_rhi_object.reset(rhi->create_sampler(this));
		return *this;
	}

	bool Sampler::archive_process(Archive& archive)
	{
		if (!Super::archive_process(archive))
		{
			return false;
		}

		archive & filter;
		archive & address_u;
		archive & address_w;
		archive & address_w;
		archive & mip_lod_bias;
		archive & anisotropy;
		archive & compare_mode;
		archive & min_lod;
		archive & max_lod;
		archive & compare_func;
		archive & unnormalized_coordinates;

		return archive;
	}

	Sampler& Sampler::apply_changes()
	{
		init_resource();
		return *this;
	}
}// namespace Engine
