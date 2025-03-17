#include <Core/reflection/class.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
	trinex_implement_engine_class(Texture, Refl::Class::IsAsset) {}

	Texture& Texture::rhi_bind(BindLocation location)
	{
		m_srv->bind(location);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, RHI_Sampler* sampler)
	{
		m_srv->bind_combined(location, sampler);
		return *this;
	}

	Texture& Texture::rhi_bind_combined(byte location, Sampler* sampler)
	{
		m_srv->bind_combined(location, sampler->rhi_sampler());
		return *this;
	}

	Texture& Texture::release_render_resources()
	{
		Super::release_render_resources();
		m_srv     = nullptr;
		return *this;
	}
}// namespace Engine
