#include <Core/reflection/render_pass_info.hpp>

namespace Engine::Refl
{
	implement_reflect_type(RenderPassInfo);

	RenderPassInfo::RenderPassInfo(Struct* parent, BitMask flags) : Super(parent, flags)
	{}

	const Vector<ShaderDefinition>& RenderPassInfo::shader_definitions() const
	{
		return m_shader_definitions;
	}

	const String& RenderPassInfo::entry() const
	{
		return m_entry;
	}

	uint_t RenderPassInfo::color_attachment_count() const
	{
		return m_color_attachments_count;
	}

	bool RenderPassInfo::has_color_attachments() const
	{
		return m_color_attachments_count > 0;
	}

	bool RenderPassInfo::has_depth_attachment() const
	{
		return m_has_depth;
	}

	bool RenderPassInfo::has_stencil_attachment() const
	{
		return m_has_stencil;
	}

	bool RenderPassInfo::has_depth_stencil_attachment() const
	{
		return m_has_depth && m_has_stencil && m_has_depth_stencil;
	}

	bool RenderPassInfo::is_material_support(const Material* material)
	{
		if (m_is_material_support)
		{
			return m_is_material_support(material);
		}
		return false;
	}
}// namespace Engine::Refl
