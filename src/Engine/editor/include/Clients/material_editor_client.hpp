#pragma once
#include <Graphics/render_viewport.hpp>
#include <imgui_windows.hpp>

namespace Engine
{
    class MaterialEditorClient : public ViewportClient
    {
        declare_class(MaterialEditorClient, ViewportClient);

    private:
        ImGuiPackageTree* _M_package_tree       = nullptr;
        ImGuiContentBrowser* _M_content_browser = nullptr;
        ImGuiObjectProperties* _M_properties    = nullptr;

        void* _M_editor_context = nullptr;

        class RenderViewport* _M_viewport         = nullptr;
        class VisualMaterial* _M_current_material = nullptr;

        bool _M_open_select_node_window = false;
        Vector2D _M_next_node_pos;
        size_t _M_frame = 0;

    public:
        void on_package_tree_close();
        void on_content_browser_close();
        void on_properties_window_close();

        MaterialEditorClient& create_package_tree();
        MaterialEditorClient& create_content_browser();
        MaterialEditorClient& create_properties_window();

        MaterialEditorClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& render(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;
        MaterialEditorClient& prepare_render(class RenderViewport* viewport) override;
        class VisualMaterial* current_material() const;


        MaterialEditorClient& render_viewport(float dt);

        void on_package_select(Package* package);
        void on_object_select(Object* object);

        void render_dock_window(void* userdata);
    };
}// namespace Engine
