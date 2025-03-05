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
		auto* self = static_class_instance();
		trinex_refl_prop(self, This, swizzle_r)->display_name("Swizze R").tooltip("Swizze R of texture");
		trinex_refl_prop(self, This, swizzle_g)->display_name("Swizze G").tooltip("Swizze G of texture");
		trinex_refl_prop(self, This, swizzle_b)->display_name("Swizze B").tooltip("Swizze B of texture");
		trinex_refl_prop(self, This, swizzle_a)->display_name("Swizze A").tooltip("Swizze A of texture");
	}


	Texture& Texture::rhi_bind_combined(Sampler* sampler, BindLocation location)
	{
		if (m_rhi_object && sampler)
		{
			rhi_object<RHI_Texture2D>()->bind_combined(sampler->rhi_object<RHI_Sampler>(), location);
		}
		return *this;
	}
}// namespace Engine
