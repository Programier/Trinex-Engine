#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Graphics/visual_material_nodes.hpp>

namespace Engine::VisualMaterialGraph
{
	trinex_implement_class(Engine::VisualMaterialGraph::MaterialRoot, 0) {}

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

	trinex_implement_class(Engine::VisualMaterialGraph::Sampler, 0)
	{
		static_node_group(static_class_instance(), "Textures");
		auto self = static_class_instance();

		trinex_refl_prop(self, This, sampler, Refl::Property::Inline);
	}
}// namespace Engine::VisualMaterialGraph
