#pragma once
#include <Graphics/render_viewport.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
    namespace ShaderCompiler
    {
        class Compiler;
        class ShaderSource;
    };// namespace ShaderCompiler

    class MaterialEditorClient : public ViewportClient
    {
        declare_class(MaterialEditorClient, ViewportClient);

    private:
        MessageList m_shader_compile_error_list;

        class ContentBrowser* m_content_browser          = nullptr;
        class ImGuiMaterialPreview* m_preview_window     = nullptr;
        class ImGuiObjectProperties* m_properties_window = nullptr;

        class RenderViewport* m_viewport             = nullptr;
        class Material* m_material                   = nullptr;
        Pointer<ShaderCompiler::Compiler> m_compiler = nullptr;

        bool m_open_select_node_window   = false;
        bool m_open_material_code_window = false;

    public:
        void on_content_browser_close();
        void on_preview_close();

        MaterialEditorClient& create_content_browser();
        MaterialEditorClient& create_preview_window();
        MaterialEditorClient& create_properties_window();
        void on_object_select(Object* object);

        MaterialEditorClient& on_bind_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;


        void render_dock_window();
        void* editor_context() const;
        MaterialEditorClient& on_object_dropped(Object* object);
        MaterialEditorClient& update_drag_and_drop();
        MaterialEditorClient& render_viewport(float dt);
        MaterialEditorClient& render(class RenderViewport* viewport) override;
    };
}// namespace Engine
