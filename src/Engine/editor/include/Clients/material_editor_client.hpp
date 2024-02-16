#pragma once
#include <Graphics/render_viewport.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
    class MaterialEditorClient : public ViewportClient
    {
        declare_class(MaterialEditorClient, ViewportClient);

    private:
        MessageList _M_shader_compile_error_list;
        class ContentBrowser* _M_content_browser      = nullptr;
        ImGuiObjectProperties* _M_properties          = nullptr;
        class ImGuiMaterialPreview* _M_preview_window = nullptr;

        void* _M_editor_context = nullptr;

        class RenderViewport* _M_viewport         = nullptr;
        class VisualMaterial* _M_current_material = nullptr;
        class ShaderCompiler* _M_compiler         = nullptr;

        bool _M_open_select_node_window   = false;
        bool _M_open_material_code_window = false;

        Vector2D _M_next_node_pos;
        size_t _M_frame = 0;

    public:
        void on_content_browser_close();
        void on_properties_window_close();
        void on_preview_close();

        MaterialEditorClient& create_content_browser();
        MaterialEditorClient& create_properties_window();
        MaterialEditorClient& create_preview_window();
        class VisualMaterial* current_material() const;
        void on_object_select(Object* object);

        MaterialEditorClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;


        void render_dock_window();
        void render_material_code();
        MaterialEditorClient& on_object_dropped(Object* object);
        MaterialEditorClient& update_drag_and_drop();
        MaterialEditorClient& render_viewport(float dt);
        MaterialEditorClient& render(class RenderViewport* viewport) override;
    };
}// namespace Engine
