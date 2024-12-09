#include <Core/reflection/render_pass_info.hpp>

namespace Engine::Refl
{
	implement_reflect_type(RenderPassInfo);

	RenderPassInfo::RenderPassInfo(Struct* parent, BitMask flags) : Super(parent, flags)
	{}

	const Vector<ShaderDefinition>& RenderPassInfo::shader_definitions() const
	{
		return m_info.shader_definitions;
	}

	const String& RenderPassInfo::entry() const
	{
		return m_info.entry;
	}

	uint_t RenderPassInfo::color_attachment_count() const
	{
		return m_info.color_attachments_count;
	}

	bool RenderPassInfo::has_color_attachments() const
	{
		return m_info.color_attachments_count > 0;
	}

	bool RenderPassInfo::has_depth_attachment() const
	{
		return m_info.has_depth;
	}

	bool RenderPassInfo::has_stencil_attachment() const
	{
		return m_info.has_stencil;
	}

	bool RenderPassInfo::has_depth_stencil_attachment() const
	{
		return m_info.has_depth && m_info.has_stencil && m_info.has_depth_stencil;
	}
}// namespace Engine::Refl
