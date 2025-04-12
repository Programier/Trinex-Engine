#include <Core/file_manager.hpp>
#include <Core/filesystem/path.hpp>
#include <Core/group.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Engine/project.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_graph.hpp>

namespace Engine
{
	static inline constexpr const char* vertex_attribute_section         = "// @trinex_vertex_attributes";
	static inline constexpr const char* global_variables_section         = "// @trinex_global_parameters";
	static inline constexpr const char* vertex_material_source_section   = "// @trinex_vertex_shader";
	static inline constexpr const char* fragment_material_source_section = "// @trinex_fragment_shader";

	trinex_implement_engine_class(VisualMaterial, Refl::Class::IsAsset) {}

	VisualMaterial::VisualMaterial()
	{
		create_node(VisualMaterialGraph::MaterialRoot::static_class_instance());
	}

	VisualMaterialGraph::Node* VisualMaterial::create_node(Refl::Class* node_class, const Vector2f& position)
	{
		if (!node_class->is_a<VisualMaterialGraph::Node>())
			return nullptr;

		VisualMaterialGraph::Node* node = Object::instance_cast<VisualMaterialGraph::Node>(node_class->create_object());
		if (node)
		{
			node->position = position;
			m_nodes.push_back(node);
		}
		return node;
	}

	// VisualMaterial& VisualMaterial::post_compile(Refl::RenderPassInfo* pass, Pipeline* pipeline)
	// {
	// 	Super::post_compile(pass, pipeline);
	// 	for (auto& node : m_nodes)
	// 	{
	// 		if (node)
	// 		{
	// 			node->override_parameter(this);
	// 		}
	// 	}
	// 	return *this;
	// }

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

			m_nodes.erase(m_nodes.begin() + index);
		}
		return *this;
	}

	static String read_material_template(MaterialDomain domain)
	{
		static auto* domain_enum = Refl::Enum::static_find("Engine::MaterialDomain", Refl::FindFlags::IsRequired);
		Name name                = domain_enum->entry(static_cast<EnumerateType>(domain))->name;

		Path file_path = Path("[shaders_dir]:/TrinexEditor/material_templates") / name.c_str() + ".slang";
		FileReader reader(file_path);

		if (reader.is_open())
		{
			return reader.read_string();
		}
		return "";
	}

	static bool compile_vertex_shader(VisualMaterialGraph::Compiler& compiler, String& template_source, size_t position,
									  VisualMaterialGraph::MaterialRoot* root, MaterialDomain domain)
	{
		static constexpr const char* format = "{0}\n"
											  "\tmaterial.position_offset = {1};\n";

		compiler.stage(VisualMaterialGraph::Compiler::Vertex);
		auto position_offset = compiler.compile(root->position_offset);

		if (!(position_offset.is_valid()))
			return false;

		String header     = compiler.compile_local_expressions();
		String out_source = Strings::format(format, header, position_offset.value);
		template_source.replace(position, std::strlen(vertex_material_source_section), out_source);
		return true;
	}

	template<typename... Args>
	static bool is_valid_expressions(const Args&... args)
	{
		return (args.is_valid() && ...);
	}

	static bool compile_fragment_shader(VisualMaterialGraph::Compiler compiler, String& template_source, size_t position,
										VisualMaterialGraph::MaterialRoot* root, MaterialDomain domain)
	{
		static constexpr const char* format = "{0}\n"
											  "\tmaterial.base_color = {1};\n"
											  "\tmaterial.emissive = {2};\n"
											  "\tmaterial.specular = {3};\n"
											  "\tmaterial.metalness = {4};\n"
											  "\tmaterial.roughness = {5};\n"
											  "\tmaterial.opacity = {6};\n"
											  "\tmaterial.AO = {7};\n"
											  "\tmaterial.normal = {8};\n"
											  "\tmaterial.position_offset = float3(0.f, 0.f, 0.f);\n";

		compiler.stage(VisualMaterialGraph::Compiler::Fragment);
		auto base_color = compiler.compile(root->base_color);
		auto emissive   = compiler.compile(root->emissive);
		auto specular   = compiler.compile(root->specular);
		auto metalness  = compiler.compile(root->metalness);
		auto roughness  = compiler.compile(root->roughness);
		auto opacity    = compiler.compile(root->opacity);
		auto AO         = compiler.compile(root->ao);
		auto normal     = compiler.compile(root->normal);

		if (!is_valid_expressions(base_color, emissive, specular, metalness, roughness, opacity, AO, normal))
			return false;

		String header     = compiler.compile_local_expressions();
		String out_source = Strings::format(format, header, base_color.value, emissive.value, specular.value, metalness.value,
											roughness.value, opacity.value, AO.value, normal.value);
		template_source.replace(position, std::strlen(fragment_material_source_section), out_source);
		return true;
	}

	bool VisualMaterial::shader_source(String& out_source)
	{
		using Root = VisualMaterialGraph::MaterialRoot;
		Root* root = instance_cast<Root>(nodes()[0].ptr());

		String template_source = read_material_template(domain);
		bool status            = true;
		VisualMaterialGraph::Compiler compiler;

		// Compile vertex shader
		{
			auto pos = template_source.find(vertex_material_source_section);
			if (pos != String::npos)
			{
				status = compile_vertex_shader(compiler, template_source, pos, root, domain);
			}
			else
			{
				status = false;
			}
		}

		// Compile fragment shader
		if (status)
		{
			auto pos = template_source.find(fragment_material_source_section);
			if (pos != String::npos)
			{
				status = compile_fragment_shader(compiler, template_source, pos, root, domain);
			}
		}

		if (status)
		{
			String globals = compiler.compile_global_expressions();
			auto pos       = template_source.find(global_variables_section);
			if (pos != String::npos)
			{
				template_source.replace(pos, std::strlen(global_variables_section), globals);
			}
			else
			{
				status = false;
			}
		}

		if (status)
		{
			out_source = std::move(template_source);
		}
		return true;
	}
}// namespace Engine
