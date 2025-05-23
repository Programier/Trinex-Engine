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

	RHI_ShaderResourceView* Texture::rhi_srv() const
	{
		if (m_texture)
			return m_texture->as_srv();
		return nullptr;
	}

	RHI_Texture* Texture::rhi_texture() const
	{
		return m_texture.get();
	}
}// namespace Engine
