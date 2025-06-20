#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>
#include <RHI/rhi_initializers.hpp>

namespace Engine::VisualMaterialGraph
{
	trinex_implement_class(Engine::VisualMaterialGraph::MaterialRoot, 0) {}

	trinex_implement_class(Engine::VisualMaterialGraph::Texture2D, 0)
	{
		static_node_group(static_class_instance(), "Textures");

		auto self = static_class_instance();
		trinex_refl_prop(self, This, name);
		trinex_refl_prop(self, This, texture);
	}

	trinex_implement_class(Engine::VisualMaterialGraph::Sampler, 0)
	{
		static_node_group(static_class_instance(), "Textures");
		auto self = static_class_instance();

		trinex_refl_prop(self, This, name);
		trinex_refl_prop(self, This, sampler, Refl::Property::Inline);
	}

	trinex_implement_class(Engine::VisualMaterialGraph::SampleTexture, 0)
	{
		static_node_group(static_class_instance(), "Textures");

		auto self = static_class_instance();
		trinex_refl_prop(self, This, texture);
		trinex_refl_prop(self, This, sampler, Refl::Property::Inline);
	}

	MaterialRoot::MaterialRoot()
	    : Node(), base_color(new_input("Base Color", RHIShaderParameterType::Float3, RHIShaderParameterType::Float3)),//
	      opacity(new_input("Opacity", RHIShaderParameterType::Float, RHIShaderParameterType::Float)),                //
	      emissive(new_input("Emissive Color", RHIShaderParameterType::Float3, RHIShaderParameterType::Float3)),      //
	      specular(new_input("Specular", RHIShaderParameterType::Float, RHIShaderParameterType::Float)),              //
	      metalness(new_input("Metalness", RHIShaderParameterType::Float, RHIShaderParameterType::Float)),            //
	      roughness(new_input("Roughness", RHIShaderParameterType::Float, RHIShaderParameterType::Float)),            //
	      ao(new_input("AO", RHIShaderParameterType::Float, RHIShaderParameterType::Float)),                          //
	      normal(new_input("Normal", RHIShaderParameterType::Float3, RHIShaderParameterType::Float3)),                //
	      position_offset(new_input("Position Offset", RHIShaderParameterType::Float3, RHIShaderParameterType::Float3))
	{
		opacity->default_value()->ref<float>()   = 1.0f;
		specular->default_value()->ref<float>()  = 0.5f;
		normal->default_value()->ref<Vector3f>() = {0.5f, 0.5f, 1.0f};
		ao->default_value()->ref<float>()        = 1.f;
	}


	Texture2D::Texture2D() : texture(DefaultResources::Textures::default_texture)
	{
		new_output("Tex", RHIShaderParameterType::Texture2D);
	}

	Texture2D* Texture2D::static_find_node(Engine::Texture2D* texture, Compiler& compiler, uint16_t id)
	{
		Node* redirected_node = compiler.find_redirection(static_class_instance(), reinterpret_cast<Identifier>(texture));

		if (auto redirected_texture = instance_cast<This>(redirected_node))
		{
			return redirected_texture;
		}

		Texture2D* texture_node = compiler.create_temp_node<This>(id);
		texture_node->texture   = texture;
		return texture_node;
	}

	Expression Texture2D::compile(OutputPin* pin, Compiler& compiler)
	{
		if (name.empty())
		{
			Node* redirected_node = compiler.find_redirection(static_class_instance(), reinterpret_cast<Identifier>(texture));

			if (auto redirected_texture = instance_cast<This>(redirected_node))
			{
				return compiler.compile(redirected_texture->outputs()[pin->index()]);
			}

			compiler.add_redirection(this, reinterpret_cast<Identifier>(texture));
		}

		return compiler.make_uniform(RHIShaderParameterType::Texture2D, name);
	}

	Texture2D& Texture2D::render()
	{
		if (texture)
		{
			float size = 4.f * ImGui::GetFrameHeight();
			ImGui::Image(texture, {size, size});
		}
		return *this;
	}

	Texture2D& Texture2D::post_compile(VisualMaterial* material)
	{
		Super::post_compile(material);
		static_post_compile(material, texture, id(), name);
		return *this;
	}

	void Texture2D::static_post_compile(VisualMaterial* material, Engine::Texture2D* texture, uint16_t id,
	                                    StringView name_override)
	{
		MaterialParameters::Parameter* parameter = nullptr;

		if (name_override.empty())
		{
			String var_name = Compiler::static_uniform_parameter_name(static_class_instance(), id);
			parameter       = material->find_parameter(var_name);
		}
		else
		{
			parameter = material->find_parameter(name_override);
		}

		if (auto texture_param = instance_cast<MaterialParameters::Texture2D>(parameter))
		{
			texture_param->texture = texture ? texture : DefaultResources::Textures::default_texture;
		}
	}

	Sampler::Sampler() : sampler(RHISamplerFilter::Point)
	{
		if (class_instance() == static_class_instance())
			new_output("Out", RHIShaderParameterType::Sampler);
	}

	Sampler* Sampler::static_find_node(const Engine::Sampler& sampler, Compiler& compiler, uint16_t id)
	{
		Node* redirected_node = compiler.find_redirection(static_class_instance(), sampler.initializer().hash());

		if (auto redirected_sampler = instance_cast<Sampler>(redirected_node))
		{
			return redirected_sampler;
		}

		Sampler* sampler_node = compiler.create_temp_node<Sampler>(id);
		sampler_node->sampler = sampler;
		return sampler_node;
	}

	Expression Sampler::compile(OutputPin* pin, Compiler& compiler)
	{
		if (name.empty())
		{
			Node* redirected_node = compiler.find_redirection(static_class_instance(), sampler.initializer().hash());

			if (auto redirected_sampler = instance_cast<Sampler>(redirected_node))
			{
				return compiler.compile(redirected_sampler->outputs()[pin->index()]);
			}

			compiler.add_redirection(this, sampler.initializer().hash());
		}

		return compiler.make_uniform(RHIShaderParameterType::Sampler, name);
	}

	Sampler& Sampler::post_compile(VisualMaterial* material)
	{
		Super::post_compile(material);
		static_post_compile(material, sampler, id());
		return *this;
	}

	void Sampler::static_post_compile(VisualMaterial* material, const Engine::Sampler& sampler, uint16_t id,
	                                  StringView name_override)
	{
		MaterialParameters::Parameter* parameter = nullptr;

		if (name_override.empty())
		{
			String var_name = Compiler::static_uniform_parameter_name(static_class_instance(), id);
			parameter       = material->find_parameter(var_name);
		}
		else
		{
			parameter = material->find_parameter(name_override);
		}

		if (auto sampler_param = instance_cast<MaterialParameters::Sampler>(parameter))
		{
			sampler_param->sampler = sampler;
		}
	}

	SampleTexture::SampleTexture() : texture(DefaultResources::Textures::default_texture), sampler(RHISamplerFilter::Point)
	{
		new_input("Tex", RHIShaderParameterType::Texture2D);
		new_input("UV", RHIShaderParameterType::Float2);
		new_input("Sampler", RHIShaderParameterType::Sampler);

		new_output("RGBA", RHIShaderParameterType::Float4);
		new_output("R", RHIShaderParameterType::Float);
		new_output("G", RHIShaderParameterType::Float);
		new_output("B", RHIShaderParameterType::Float);
		new_output("A", RHIShaderParameterType::Float);
	}

	Expression SampleTexture::compile_texture(Compiler& compiler)
	{
		auto in_texture = texture_pin();

		if (in_texture->linked_pin())
			return compiler.compile(in_texture);

		Texture2D* node = Texture2D::static_find_node(texture, compiler, id());
		return compiler.compile(node->texture_pin());
	}

	Expression SampleTexture::compile_uv(Compiler& compiler)
	{
		auto in_uv = uv_pin();

		if (in_uv->linked_pin())
			return compiler.compile(in_uv);

		return Expression(RHIShaderParameterType::Float2, "input.uv");
	}

	Expression SampleTexture::compile_sampler(Compiler& compiler)
	{
		auto in_sampler = sampler_pin();

		if (in_sampler->linked_pin())
			return compiler.compile(in_sampler);

		Sampler* sampler_node = Sampler::static_find_node(sampler, compiler, id());
		return compiler.compile(sampler_node->sampler_pin());
	}

	Engine::Texture2D* SampleTexture::find_texture()
	{
		Node* node = this;

		do
		{
			InputPin* pin = nullptr;

			for (InputPin* input : node->inputs())
			{
				if (input->type() == RHIShaderParameterType::Texture2D)
				{
					pin = input;
					break;
				}
			}

			if (pin && pin->linked_pin())
			{
				node = pin->linked_pin()->node();
			}
			else
			{
				node = nullptr;
			}

		} while (node && !node->is_instance_of<Texture2D>());

		if (auto texture = instance_cast<Texture2D>(node))
		{
			return texture->texture;
		}

		return texture;
	}

	Expression SampleTexture::compile(OutputPin* pin, Compiler& compiler)
	{
		const size_t pin_index = pin->index();

		if (pin_index != 0)
		{
			Expression result     = compiler.compile(outputs()[0]);
			const char* swizzle[] = {".r", ".g", ".b", ".a"};

			result.value += swizzle[pin_index - 1];
			result.type = RHIShaderParameterType::Float;
			return result;
		}

		Expression texture = compile_texture(compiler);

		if (!texture.is_valid())
			return Expression();

		Expression uv = compile_uv(compiler);

		if (!uv.is_valid())
			return Expression();

		Expression sampler = compile_sampler(compiler);

		if (!sampler.is_valid())
			return Expression();

		String expr = Strings::format("{}.Sample({}, {})", texture.value, sampler.value, uv.value);
		return compiler.make_variable(Expression(RHIShaderParameterType::Float4, expr));
	}

	SampleTexture& SampleTexture::render()
	{
		Engine::Texture2D* icon = find_texture();

		if (icon)
		{
			float size = 4.f * ImGui::GetFrameHeight();
			ImGui::Image(icon, {size, size});
		}

		return *this;
	}

	SampleTexture& SampleTexture::post_compile(VisualMaterial* material)
	{
		Super::post_compile(material);

		if (!texture_pin()->linked_pin())
		{
			Texture2D::static_post_compile(material, texture, id());
		}

		if (!sampler_pin()->linked_pin())
		{
			Sampler::static_post_compile(material, sampler, id());
		}

		return *this;
	}
}// namespace Engine::VisualMaterialGraph
