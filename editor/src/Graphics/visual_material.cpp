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

	static inline constexpr size_t base_color_index      = 0;
	static inline constexpr size_t opacity_index         = 1;
	static inline constexpr size_t emissive_index        = 2;
	static inline constexpr size_t specular_index        = 3;
	static inline constexpr size_t metalness_index       = 4;
	static inline constexpr size_t roughness_index       = 5;
	static inline constexpr size_t ao_index              = 6;
	static inline constexpr size_t normal_index          = 7;
	static inline constexpr size_t position_offset_index = 8;

	trinex_implement_engine_class(VisualMaterial, Refl::Class::IsAsset | Refl::Class::IsScriptable)
	{
		auto* self = This::static_class_instance();
		trinex_refl_prop(self, This, domain);
	}


	VisualMaterial::VisualMaterial() : domain(MaterialDomain::Surface)
	{
		create_node(Refl::Class::static_require("Engine::VisualMaterialGraph::Root"));
	}

	const Vector<Pointer<VisualMaterialGraph::Node>>& VisualMaterial::nodes() const
	{
		return m_nodes;
	}

	VisualMaterial& VisualMaterial::register_node(VisualMaterialGraph::Node* node)
	{
		if (node && (!node->is_root_node() || m_nodes.empty()))
			m_nodes.push_back(node);
		return *this;
	}

	VisualMaterialGraph::Node* VisualMaterial::create_node(Refl::Class* node_class, const Vector2f& position)
	{
		if (node_class->is_a<VisualMaterialGraph::Node>())
		{
			VisualMaterialGraph::Node* node = Object::instance_cast<VisualMaterialGraph::Node>(node_class->create_object());
			if (node)
			{
				node->position = position;
				register_node(node);
			}
			return node;
		}
		return nullptr;
	}

	VisualMaterial& VisualMaterial::post_compile(Refl::RenderPassInfo* pass, Pipeline* pipeline)
	{
		Super::post_compile(pass, pipeline);
		for (auto& node : m_nodes)
		{
			if (node)
			{
				node->override_parameter(this);
			}
		}
		return *this;
	}

	VisualMaterial& VisualMaterial::destroy_node(VisualMaterialGraph::Node* node, bool destroy_links)
	{
		if (!node->is_root_node())
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

	static bool compile_vertex_shader(VisualMaterialGraph::GlobalCompilerState& gs, String& template_source, size_t position,
	                                  VisualMaterialGraph::Node* root, MaterialDomain domain)
	{
		VisualMaterialGraph::CompilerState compiler(gs);


		static constexpr const char* format = "{0}\n"
											  "\tmaterial.position_offset = {1};\n";

		auto position_offset = root->compile(root->inputs()[position_offset_index], compiler);

		if (!(position_offset.is_valid()))
			return false;

		String header     = compiler.create_header("\t");
		String out_source = Strings::format(format, header, position_offset.code);
		template_source.replace(position, std::strlen(vertex_material_source_section), out_source);
		return true;
	}


	template<typename... Args>
	static bool is_valid_expressions(const Args&... args)
	{
		return (args.is_valid() && ...);
	}

	static bool compile_fragment_shader(VisualMaterialGraph::GlobalCompilerState& gs, String& template_source, size_t position,
	                                    VisualMaterialGraph::Node* root, MaterialDomain domain)
	{
		VisualMaterialGraph::CompilerState compiler(gs);

		static constexpr const char* format = "{0}\n"
											  "\tmaterial.base_color = {1};\n"
											  "\tmaterial.emissive = {2};\n"
											  "\tmaterial.normal = input.world_normal;\n"
											  "\tmaterial.position_offset   = float3(0.f, 0.f, 0.f);\n"
											  "\tmaterial.specular = {3};\n"
											  "\tmaterial.metalness = {4};\n"
											  "\tmaterial.roughness = {5};\n"
											  "\tmaterial.opacity = {6};\n"
											  "\tmaterial.AO = {7};\n"
											  "\tmaterial.normal = {8};\n";

		auto base_color = root->compile(root->inputs()[base_color_index], compiler);
		auto emissive   = root->compile(root->inputs()[emissive_index], compiler);
		auto specular   = root->compile(root->inputs()[specular_index], compiler);
		auto metalness  = root->compile(root->inputs()[metalness_index], compiler);
		auto roughness  = root->compile(root->inputs()[roughness_index], compiler);
		auto opacity    = root->compile(root->inputs()[opacity_index], compiler);
		auto AO         = root->compile(root->inputs()[ao_index], compiler);
		auto normal     = root->compile(root->inputs()[normal_index], compiler);

		if (!is_valid_expressions(base_color, emissive, specular, metalness, roughness, opacity, AO, normal))
			return false;

		String header     = compiler.create_header("\t");
		String out_source = Strings::format(format, header, base_color.code, emissive.code, specular.code, metalness.code,
											roughness.code, opacity.code, AO.code, normal.code);
		template_source.replace(position, std::strlen(fragment_material_source_section), out_source);
		return true;
	}

	bool VisualMaterial::shader_source(String& out_source)
	{
		String template_source = read_material_template(domain);
		bool status            = true;
		VisualMaterialGraph::GlobalCompilerState global_state;

		// Compile vertex shader
		{
			auto pos = template_source.find(vertex_material_source_section);
			if (pos != String::npos)
			{
				status = compile_vertex_shader(global_state, template_source, pos, nodes()[0], domain);
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
				status = compile_fragment_shader(global_state, template_source, pos, nodes()[0], domain);
			}
		}

		if (status)
		{
			String globals = global_state.compile();
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

		return status;
	}
}// namespace Engine
