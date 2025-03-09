#include <Core/reflection/class.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture.hpp>

namespace Engine
{
	trinex_implement_engine_class(Texture, Refl::Class::IsAsset) {}

	RHI_UnorderedAccessView* Texture::rhi_unordered_access_view() const
	{
		if (!m_uav && m_texture)
		{
			m_uav = m_texture->create_uav();
		}
		return m_uav;
	}

	Texture& Texture::release_render_resources()
	{
		Super::release_render_resources();
		m_uav     = nullptr;
		m_srv     = nullptr;
		m_texture = nullptr;
		return *this;
	}
}// namespace Engine
