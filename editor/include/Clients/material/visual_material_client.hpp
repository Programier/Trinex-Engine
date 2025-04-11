#include <Clients/material/material_client.hpp>
#include <Widgets/property_renderer.hpp>

namespace ax::NodeEditor
{
	class EditorContext;
}

namespace Engine
{
	namespace VisualMaterialGraph
	{
		class Node;
		class Pin;
	}// namespace VisualMaterialGraph

	class VisualMaterial;

	class VisualMaterialEditorClient : public MaterialEditorClient
	{
		trinex_declare_class(VisualMaterialEditorClient, MaterialEditorClient);

	private:
		PropertyRenderer m_property_renderer;
		ax::NodeEditor::EditorContext* m_context = nullptr;
		Pointer<VisualMaterial> m_material;

		struct {
			String filter                 = "";
			Vector2f pos                  = {0.f, 0.f};
			VisualMaterialGraph::Pin* pin = nullptr;
			bool is_active                = false;
		} m_create_node_ctx;

		VisualMaterialEditorClient& on_node_select(VisualMaterialGraph::Node* node);
		VisualMaterialEditorClient& on_node_create(VisualMaterialGraph::Node* node);
		VisualMaterialEditorClient& on_node_destroy(VisualMaterialGraph::Node* node);

		VisualMaterialEditorClient& open_spawn_node_window(VisualMaterialGraph::Pin* pin = nullptr);

		VisualMaterialEditorClient& render_graph();
		bool render_properties(VisualMaterialGraph::Node* node);
		VisualMaterialEditorClient& render_spawn_node_window();

		VisualMaterialEditorClient& update_create_events();
		VisualMaterialEditorClient& update_delete_events();
		VisualMaterialEditorClient& update_events();

	public:
		VisualMaterialEditorClient();
		~VisualMaterialEditorClient();

		VisualMaterialEditorClient& update(float dt) override;
		VisualMaterialEditorClient& select(Object* object) override;
		uint32_t build_dock(uint32_t dock) override;
	};
}// namespace Engine
