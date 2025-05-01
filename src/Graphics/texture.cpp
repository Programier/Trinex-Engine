#include <Core/reflection/class.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
	trinex_implement_engine_class(Texture, Refl::Class::IsAsset) {}

	Texture& Texture::rhi_bind(byte location)
	{
		rhi->bind_srv(rhi_srv(), location);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, RHI_Sampler* sampler)
	{
		rhi->bind_srv(rhi_srv(), location, sampler);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, Sampler* sampler)
	{
		rhi->bind_srv(rhi_srv(), location, sampler->rhi_sampler());
		return *this;
	}
}// namespace Engine
