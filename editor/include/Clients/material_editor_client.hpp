#pragma once
#include <Clients/imgui_client.hpp>
#include <Widgets/imgui_windows.hpp>


namespace ax::NodeEditor
{
	class EditorContext;
}

namespace Engine
{
	namespace ShaderCompiler
	{
		class Compiler;
		struct ShaderSource;
	};// namespace ShaderCompiler

	namespace VisualMaterialGraph
	{
		class Node;
	}

	class MaterialEditorClient : public ImGuiEditorClient
	{
		declare_class(MaterialEditorClient, ImGuiEditorClient);

	public:
		struct GraphState {
			Set<Pointer<VisualMaterialGraph::Node>, Pointer<VisualMaterialGraph::Node>::HashStruct> m_nodes;

			Vector2D m_node_spawn_position;
			void* m_create_node_from_pin = nullptr;
			String m_nodes_filter        = "";
		};

	private:
		MessageList m_shader_compile_error_list;

		class ContentBrowser* m_content_browser             = nullptr;
		class ImGuiMaterialPreview* m_preview_window        = nullptr;
		class ImGuiObjectProperties* m_properties_window    = nullptr;
		class ImGuiNodeProperties* m_node_properties_window = nullptr;
		class ImGuiMaterialCode* m_material_code            = nullptr;

		GraphState m_graph_state;
		ax::NodeEditor::EditorContext* m_graph_editor_context = nullptr;
		class Material* m_material                            = nullptr;
		class VisualMaterialGraph::Node* m_selected_node      = nullptr;
		Pointer<ShaderCompiler::Compiler> m_compiler          = nullptr;

		bool m_open_select_node_window   = false;
		bool m_is_open_create_node_popup = false;

		MaterialEditorClient& select(Object* object) override;
		void on_node_select(Object* object);
		void on_object_dropped(Object* object);

		MaterialEditorClient& process_editor_events(class VisualMaterial* material);
		bool show_new_node_popup(class VisualMaterial* material);

	public:
		MaterialEditorClient();
		~MaterialEditorClient();

		MaterialEditorClient& create_content_browser();
		MaterialEditorClient& create_preview_window();
		MaterialEditorClient& create_material_code_window();
		MaterialEditorClient& create_properties_window();
		MaterialEditorClient& create_node_properties_window();

		MaterialEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		MaterialEditorClient& update(float dt) override;

		void render_dock_window();
		void* editor_context() const;
		MaterialEditorClient& update_drag_and_drop();
		MaterialEditorClient& render_viewport(float dt);
		MaterialEditorClient& render_visual_material_graph(class VisualMaterial* material);
	};
}// namespace Engine
