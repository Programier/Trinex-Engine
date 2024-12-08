#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
	implement_engine_class(Texture, Refl::Class::IsAsset)
	{
		auto* self         = static_class_instance();
		auto* swizzle_enum = Refl::Enum::static_find("Engine::Swizzle", Refl::FindFlags::IsRequired);

		trinex_refl_prop(self, This, swizzle_r, swizzle_enum)->display_name("Swizze R").tooltip("Swizze R of texture");
		trinex_refl_prop(self, This, swizzle_g, swizzle_enum)->display_name("Swizze G").tooltip("Swizze G of texture");
		trinex_refl_prop(self, This, swizzle_b, swizzle_enum)->display_name("Swizze B").tooltip("Swizze B of texture");
		trinex_refl_prop(self, This, swizzle_a, swizzle_enum)->display_name("Swizze A").tooltip("Swizze A of texture");
	}


	Texture& Texture::rhi_bind_combined(Sampler* sampler, BindLocation location)
	{
		if (m_rhi_object && sampler)
		{
			rhi_object<RHI_Texture>()->bind_combined(sampler->rhi_object<RHI_Sampler>(), location);
		}
		return *this;
	}
}// namespace Engine
