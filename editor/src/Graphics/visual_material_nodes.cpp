#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/texture_2D.hpp>
#include <Graphics/visual_material.hpp>
#include <Graphics/visual_material_nodes.hpp>

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

		trinex_refl_prop(self, This, sampler, Refl::Property::Inline);
	}

	trinex_implement_class(Engine::VisualMaterialGraph::SampleTexture, 0)
	{
		static_node_group(static_class_instance(), "Textures");

		auto self = static_class_instance();
		trinex_refl_prop(self, This, sampler, Refl::Property::Inline);
	}

	MaterialRoot::MaterialRoot()
	    : Node(), base_color(new_input("Base Color", ShaderParameterType::Float3, ShaderParameterType::Float3)),//
	      opacity(new_input("Opacity", ShaderParameterType::Float, ShaderParameterType::Float)),                //
	      emissive(new_input("Emissive Color", ShaderParameterType::Float3, ShaderParameterType::Float3)),      //
	      specular(new_input("Specular", ShaderParameterType::Float, ShaderParameterType::Float)),              //
	      metalness(new_input("Metalness", ShaderParameterType::Float, ShaderParameterType::Float)),            //
	      roughness(new_input("Roughness", ShaderParameterType::Float, ShaderParameterType::Float)),            //
	      ao(new_input("AO", ShaderParameterType::Float, ShaderParameterType::Float)),                          //
	      normal(new_input("Normal", ShaderParameterType::Float3, ShaderParameterType::Float3)),                //
	      position_offset(new_input("Position Offset", ShaderParameterType::Float3, ShaderParameterType::Float3))
	{
		opacity->default_value()->ref<float>()   = 1.0f;
		specular->default_value()->ref<float>()  = 0.5f;
		normal->default_value()->ref<Vector3f>() = {0.5f, 0.5f, 1.0f};
		ao->default_value()->ref<float>()        = 1.f;
	}


	Texture2D::Texture2D() : texture(DefaultResources::Textures::default_texture)
	{
		new_output("Out", ShaderParameterType::Texture2D);
	}

	Expression Texture2D::compile(OutputPin* pin, Compiler& compiler)
	{
		return compiler.make_uniform(ShaderParameterType::Texture2D, name);
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

		MaterialParameters::Parameter* parameter = nullptr;

		if (name.empty())
		{
			String var_name = Compiler::static_uniform_parameter_name(this);
			parameter       = material->find_parameter(var_name);
		}
		else
		{
			parameter = material->find_parameter(name);
		}

		if (auto texture_param = instance_cast<MaterialParameters::Texture2D>(parameter))
		{
			texture_param->texture = texture ? texture : DefaultResources::Textures::default_texture;
		}

		return *this;
	}

	Sampler::Sampler()
	{
		new_output("Out", ShaderParameterType::Sampler);
	}

	Expression Sampler::compile(OutputPin* pin, Compiler& compiler)
	{
		return compiler.make_uniform(ShaderParameterType::Sampler);
	}

	SampleTexture::SampleTexture()
	{
		new_input("Tex", ShaderParameterType::META_Texture);
		new_input("UV", ShaderParameterType::META_Numeric);
		new_input("Sampler", ShaderParameterType::Sampler);
		new_output("Out", ShaderParameterType::Float4);
	}

	Expression SampleTexture::compile(OutputPin* pin, Compiler& compiler)
	{
		auto in_uv = uv_pin();

		Expression texture = compiler.compile(texture_pin());
		Expression sampler = compiler.compile(sampler_pin());

		if (texture.type == ShaderParameterType::Texture2D)
		{
			Expression uv;

			if (in_uv->linked_pin())
				uv = compiler.compile(in_uv).convert(ShaderParameterType::Float2);
			else
				uv = Expression(ShaderParameterType::Float2, "input.uv");

			if (uv.is_valid())
			{
				String expr = Strings::format("{}.Sample({}, {})", texture.value, sampler.value, uv.value);
				return Expression(ShaderParameterType::Float4, expr);
			}
		}

		return Expression();
	}
}// namespace Engine::VisualMaterialGraph
