#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/enum.hpp>
#include <Core/implement.hpp>
#include <Core/logger.hpp>
#include <Core/property.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>
#include <Image/image.hpp>

namespace Engine
{
	implement_engine_class(Texture, Class::IsAsset)
	{
		Class* self		   = static_class_instance();
		Enum* swizzle_enum = Enum::static_find("Engine::Swizzle");
		self->add_properties(new EnumProperty("Swizze R", "Swizze R of texture", &This::swizzle_r, swizzle_enum),
							 new EnumProperty("Swizze G", "Swizze G of texture", &This::swizzle_g, swizzle_enum),
							 new EnumProperty("Swizze B", "Swizze B of texture", &This::swizzle_b, swizzle_enum),
							 new EnumProperty("Swizze A", "Swizze A of texture", &This::swizzle_a, swizzle_enum));
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
