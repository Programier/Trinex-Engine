#include <Core/exception.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Core/string_functions.hpp>

namespace Engine::Refl
{
	RenderPassInfo* RenderPassInfo::s_head = nullptr;
	RenderPassInfo* RenderPassInfo::s_tail = nullptr;

	static Map<Name, RenderPassInfo*, Name::HashFunction> s_render_pass_table;

	implement_reflect_type(RenderPassInfo);

	RenderPassInfo* RenderPassInfo::static_find_pass(Name name)
	{
		auto it = s_render_pass_table.find(name);
		return it == s_render_pass_table.end() ? nullptr : it->second;
	}

	RenderPassInfo::RenderPassInfo(Struct* parent, BitMask flags) : Super(parent, flags)
	{}

	RenderPassInfo::~RenderPassInfo()
	{
		s_render_pass_table.erase(name());
	}

	RenderPassInfo& RenderPassInfo::initialize()
	{
		Super::initialize();

		if (name() == "Default")
			throw EngineException("Cannot create render pass with name 'Default'. This name is reserved by Trinex Engine");

		if (s_render_pass_table.contains(name()))
			throw EngineException(Strings::format("Render Pass with name '{}' already registered!", name().c_str()));

		s_render_pass_table.insert({name(), this});

		if (s_head == nullptr)
		{
			s_head = s_tail = this;
		}
		else
		{
			s_tail->m_next = this;
			s_tail         = this;
		}
		return *this;
	}

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

	bool RenderPassInfo::is_material_compatible(const Material* material)
	{
		if (m_is_material_compatible)
		{
			return m_is_material_compatible(material);
		}
		return false;
	}
}// namespace Engine::Refl
