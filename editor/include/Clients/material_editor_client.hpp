#pragma once
#include <Graphics/render_viewport.hpp>
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

	class MaterialEditorClient : public ViewportClient
	{
		declare_class(MaterialEditorClient, ViewportClient);

	public:
		struct GraphState {
			Vector2D m_node_spawn_position;
		};

	private:
		MessageList m_shader_compile_error_list;

		class ContentBrowser* m_content_browser          = nullptr;
		class ImGuiMaterialPreview* m_preview_window     = nullptr;
		class ImGuiObjectProperties* m_properties_window = nullptr;
		class ImGuiMaterialCode* m_material_code         = nullptr;

		ax::NodeEditor::EditorContext* m_graph_editor_context = nullptr;
		GraphState m_graph_state;

		class RenderViewport* m_viewport             = nullptr;
		class Material* m_material                   = nullptr;
		Pointer<ShaderCompiler::Compiler> m_compiler = nullptr;

		bool m_open_select_node_window = false;

		// Graph editor state
		bool m_is_open_create_node_popup = false;
		void* m_create_node_from_pin     = nullptr;

	public:
		MaterialEditorClient();
		~MaterialEditorClient();

		MaterialEditorClient& create_content_browser();
		MaterialEditorClient& create_preview_window();
		MaterialEditorClient& create_material_code_window();
		MaterialEditorClient& create_properties_window();
		void on_object_select(Object* object);

		MaterialEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		MaterialEditorClient& on_unbind_viewport(class RenderViewport* viewport) override;
		MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;


		void render_dock_window();
		void* editor_context() const;
		MaterialEditorClient& on_object_dropped(Object* object);
		MaterialEditorClient& update_drag_and_drop();
		MaterialEditorClient& render_viewport(float dt);
		MaterialEditorClient& render_visual_material_graph(class VisualMaterial* material);
		MaterialEditorClient& render(class RenderViewport* viewport) override;
	};
}// namespace Engine
