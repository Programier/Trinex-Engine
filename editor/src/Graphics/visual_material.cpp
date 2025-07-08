#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/group.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>
#include <limits>

namespace Engine
{
	static constexpr const char* s_material_template = R"(
import "trinex/material.slang";
import "trinex/scene_view.slang";

// Global statemets
{0}

export Material main<Material : IMaterial>(in IMaterialInput input)
{{
    {1}

    Material material;
    material.base_color      = {2};
    material.emissive        = {3};
    material.specular        = {4};
    material.metalness       = {5};
    material.roughness       = {6};
    material.opacity         = {7};
    material.ao              = {8};
    material.normal          = {9};
    material.position_offset = {10};
    return material;
}}

)";

	trinex_implement_engine_class(VisualMaterial, Refl::Class::IsAsset) {}

	VisualMaterial::VisualMaterial()
	{
		create_node(VisualMaterialGraph::MaterialRoot::static_class_instance());
	}

	VisualMaterial& VisualMaterial::recalculate_nodes_ids()
	{
		m_next_node_id = 0;

		for (VisualMaterialGraph::Node* node : m_nodes)
		{
			node->change_id(m_next_node_id++);
		}
		return *this;
	}

	VisualMaterialGraph::Node* VisualMaterial::create_node(Refl::Class* node_class, const Vector2f& position)
	{
		if (m_nodes.size() == std::numeric_limits<uint16_t>::max())
		{
			error_log("VisualMaterialGraph",
			          "The limit of %d nodes has been reached. Take a look at the material — you're "
			          "clearly doing something wrong...",
			          std::numeric_limits<uint16_t>::max());

			return nullptr;
		}

		if (!node_class->is_a<VisualMaterialGraph::Node>())
			return nullptr;

		VisualMaterialGraph::Node* node = Object::instance_cast<VisualMaterialGraph::Node>(node_class->create_object("", this));

		if (node)
		{
			node->position = position;
			m_nodes.push_back(node);

			if (m_next_node_id == std::numeric_limits<uint16_t>::max())
			{
				recalculate_nodes_ids();
			}
			else
			{
				node->change_id(m_next_node_id++);
			}
		}

		return node;
	}

	VisualMaterial& VisualMaterial::post_compile(RenderPass* pass, GraphicsPipeline* pipeline)
	{
		Super::post_compile(pass, pipeline);
		for (auto& node : m_nodes) node->post_compile(this);
		return *this;
	}

	VisualMaterial::~VisualMaterial()
	{
		for (auto& node : m_nodes)
		{
			if (node->owner() == this)
				node->owner(nullptr);
		}
	}

	VisualMaterial& VisualMaterial::destroy_node(VisualMaterialGraph::Node* node, bool destroy_links)
	{
		if (node != m_nodes[0].ptr())
		{
			size_t index = 0;
			for (size_t count = m_nodes.size(); index < count; ++index)
			{
				if (m_nodes[index].ptr() == node)
					break;
			}

			if (index == m_nodes.size())
				return *this;

			if (destroy_links)
			{
				for (VisualMaterialGraph::InputPin* in : node->inputs())
				{
					in->unlink();
				}

				for (VisualMaterialGraph::OutputPin* out : node->outputs())
				{
					out->unlink();
				}
			}

			if (node->owner() == this)
				node->owner(nullptr);

			m_nodes.erase(m_nodes.begin() + index);
		}
		return *this;
	}

	template<typename... Args>
	static bool is_valid_expressions(const Args&... args)
	{
		return (args.is_valid() && ...);
	}

	bool VisualMaterial::shader_source(String& out_source)
	{
		auto root = instance_cast<VisualMaterialGraph::MaterialRoot>(nodes()[0].ptr());

		VisualMaterialGraph::Compiler compiler;

		auto base_color      = compiler.compile(root->base_color);
		auto emissive        = compiler.compile(root->emissive);
		auto specular        = compiler.compile(root->specular);
		auto metalness       = compiler.compile(root->metalness);
		auto roughness       = compiler.compile(root->roughness);
		auto opacity         = compiler.compile(root->opacity);
		auto AO              = compiler.compile(root->ao);
		auto normal          = compiler.compile(root->normal);
		auto position_offset = compiler.compile(root->position_offset);

		if (!is_valid_expressions(base_color, emissive, specular, metalness, roughness, opacity, AO, normal, position_offset))
			return false;

		String locals  = compiler.compile_local_expressions();
		String globals = compiler.compile_global_expressions();

		out_source =
		        Strings::format(s_material_template, globals, locals, base_color.value, emissive.value, specular.value,
		                        metalness.value, roughness.value, opacity.value, AO.value, normal.value, position_offset.value);
		return true;
	}
}// namespace Engine
