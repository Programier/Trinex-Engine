#include <Clients/material/material_client.hpp>

namespace ax::NodeEditor
{
	class EditorContext;
}

namespace Engine
{
	namespace VisualMaterialGraph
	{
		class Node;
	}

	class VisualMaterial;

	class VisualMaterialEditorClient : public MaterialEditorClient
	{
		trinex_declare_class(VisualMaterialEditorClient, MaterialEditorClient);

	public:
		struct GraphState {
			Vector2f m_node_spawn_position;
			void* m_create_node_from_pin = nullptr;
		};

	private:
		ax::NodeEditor::EditorContext* m_context = nullptr;
		Pointer<VisualMaterial> m_material;
		String m_nodes_filter = "";

		GraphState m_graph_state;

		VisualMaterialEditorClient& open_spawn_node_window();
		VisualMaterialEditorClient& render_spawn_node_window();
		VisualMaterialEditorClient& update_create_events();
		VisualMaterialEditorClient& update_delete_events();
		VisualMaterialEditorClient& update_events();
		VisualMaterialEditorClient& on_node_select(VisualMaterialGraph::Node* node);
		VisualMaterialEditorClient& render_graph();

	public:
		VisualMaterialEditorClient();
		~VisualMaterialEditorClient();

		VisualMaterialEditorClient& update(float dt) override;
		VisualMaterialEditorClient& select(Object* object) override;
		uint32_t build_dock(uint32_t dock) override;
	};
}// namespace Engine
