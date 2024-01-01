#pragma once
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <imgui_windows.hpp>

namespace Engine
{
    class EditorViewportClient : public ViewportClient
    {

        declare_class(EditorViewportClient, ViewportClient);

    private:
        ScriptObject _M_script_object;
        ImGuiPackageTree _M_package_tree;
        ImGuiContentBrowser _M_content_browser;
        ImGuiObjectProperties _M_properties;

        size_t _M_frame = 0;

    public:
        EditorViewportClient();

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;
        ViewportClient& prepare_render(class RenderViewport* viewport) override;
        ViewportClient& destroy_script_object(ScriptObject* object) override;


        EditorViewportClient& init_world();
        EditorViewportClient& create_properties_window(float dt);
        EditorViewportClient& create_log_window(float dt);
        EditorViewportClient& create_viewport_window(float dt);
        void render_dock_window(void* userdata);

        void on_package_select(Package* package);
        void on_object_select(Object* object);
    };
}// namespace Engine
