#pragma once
#include <Core/flags.hpp>
#include <Core/pointer.hpp>
#include <Graphics/material.hpp>

namespace Engine
{
	namespace VisualMaterialGraph
	{
		class Node;
		class NodeLayout;
		class MaterialRoot;
	}// namespace VisualMaterialGraph

	class VisualMaterial : public Material
	{
		trinex_declare_class(VisualMaterial, Material);

	private:
		using RootNode = VisualMaterialGraph::MaterialRoot;

		Vector<Pointer<VisualMaterialGraph::Node>> m_nodes;
		uint16_t m_next_node_id = 0;

		VisualMaterial& recalculate_nodes_ids();

	public:
		VisualMaterial();
		VisualMaterialGraph::Node* create_node(class Refl::Class* node_class, const Vector2f& position = {});
		VisualMaterial& destroy_node(VisualMaterialGraph::Node* node, bool destroy_links = true);
		// VisualMaterial& post_compile(RenderPass* pass, GraphicsPipeline* pipeline) override;
		~VisualMaterial();

		template<typename T>
		T* create_node()
		{
			static_assert(std::is_base_of_v<VisualMaterialGraph::Node, T>,
			              "Template type must have base class VisualMaterialGraph::Node");
			return Object::instance_cast<T>(create_node(T::static_class_instance()));
		}

		bool shader_source(String& out_source) override;

		inline const Vector<Pointer<VisualMaterialGraph::Node>>& nodes() const { return m_nodes; }
		inline RootNode* root_node() const { return reinterpret_cast<RootNode*>(nodes()[0].ptr()); }
		inline bool is_root_node(VisualMaterialGraph::Node* node) const { return m_nodes[0].ptr() == node; }
	};
}// namespace Engine
