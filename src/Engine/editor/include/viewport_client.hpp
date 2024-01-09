#pragma once
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
        ImGuiPackageTree* _M_package_tree;
        ImGuiContentBrowser* _M_content_browser;
        ImGuiObjectProperties* _M_properties;
        ImGuiSceneTree* _M_scene_tree;

        class Sampler* _M_sampler = nullptr;

        size_t _M_frame = 0;

    public:
        EditorViewportClient();

        void on_package_tree_close();
        void on_content_browser_close();
        void on_properties_window_close();
        void on_scene_tree_close();

        EditorViewportClient& create_package_tree();
        EditorViewportClient& create_content_browser();
        EditorViewportClient& create_properties_window();
        EditorViewportClient& create_scene_tree();

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;
        ViewportClient& prepare_render(class RenderViewport* viewport) override;
        ViewportClient& destroy_script_object(ScriptObject* object) override;


        EditorViewportClient& init_world();
        EditorViewportClient& create_log_window(float dt);
        EditorViewportClient& create_viewport_window(float dt);
        void render_dock_window(void* userdata);

        void on_package_select(Package* package);
        void on_object_select(Object* object);
    };
}// namespace Engine
