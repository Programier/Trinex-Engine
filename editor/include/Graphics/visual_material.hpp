#pragma once
#include <Core/flags.hpp>
#include <Core/pointer.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
	namespace VisualMaterialGraph
	{
		class Node;
	}

	class VisualMaterial : public Material
	{
		declare_class(VisualMaterial, Material);

	private:
		Vector<Pointer<VisualMaterialGraph::Node>> m_nodes;

	public:
		MaterialDomain domain;

		VisualMaterial();
		const Vector<Pointer<VisualMaterialGraph::Node>>& nodes() const;
		VisualMaterial& register_node(VisualMaterialGraph::Node* node);
		VisualMaterialGraph::Node* create_node(class Refl::Class* node_class, const Vector2D& position = {});

		template<typename T>
		T* create_node()
		{
			static_assert(std::is_base_of_v<VisualMaterialGraph::Node, T>,
			              "Template type must have base class VisualMaterialGraph::Node");
			return Object::instance_cast<T>(create_node(T::static_class_instance()));
		}

		VisualMaterial& destroy_node(VisualMaterialGraph::Node* node, bool destroy_links = true);
		bool shader_source(String& out_source) override;
		bool compile(ShaderCompiler::Compiler* compiler = nullptr) override;
	};
}// namespace Engine
