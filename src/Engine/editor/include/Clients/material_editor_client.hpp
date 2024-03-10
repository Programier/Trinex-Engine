#pragma once
#include <Graphics/render_viewport.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
    class MaterialEditorClient : public ViewportClient
    {
        declare_class(MaterialEditorClient, ViewportClient);

    private:
        MessageList m_shader_compile_error_list;
        class ContentBrowser* m_content_browser      = nullptr;
        ImGuiObjectProperties* m_properties          = nullptr;
        class ImGuiMaterialPreview* m_preview_window = nullptr;

        void* m_editor_context = nullptr;

        class RenderViewport* m_viewport = nullptr;
        class ShaderCompiler* m_compiler = nullptr;

        bool m_open_select_node_window   = false;
        bool m_open_material_code_window = false;

        Vector2D m_next_node_pos;
        size_t m_frame = 0;

    public:
        void on_content_browser_close();
        void on_properties_window_close();
        void on_preview_close();

        MaterialEditorClient& create_content_browser();
        MaterialEditorClient& create_properties_window();
        MaterialEditorClient& create_preview_window();
        void on_object_select(Object* object);

        MaterialEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;


        void render_dock_window();
        void render_material_code();
        void* editor_context() const;
        MaterialEditorClient& on_object_dropped(Object* object);
        MaterialEditorClient& update_drag_and_drop();
        MaterialEditorClient& render_viewport(float dt);
        MaterialEditorClient& render(class RenderViewport* viewport) override;
        ImGuiObjectProperties* properties_window() const;
    };
}// namespace Engine
