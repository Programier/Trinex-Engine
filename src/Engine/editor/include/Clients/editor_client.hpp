#pragma once
#include <Engine/scene_renderer.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
    class EditorClient : public ViewportClient
    {

        declare_class(EditorClient, ViewportClient);

    private:
        SceneRenderer _M_renderer;

        ScriptObject _M_script_object;
        class ContentBrowser* _M_content_browser;
        ImGuiObjectProperties* _M_properties;
        ImGuiSceneTree* _M_scene_tree;
        TexCoordVertexBuffer* mesh;

        class Sampler* _M_sampler = nullptr;
        size_t _M_frame           = 0;

    public:
        EditorClient();

        void on_content_browser_close();
        void on_properties_window_close();
        void on_scene_tree_close();

        EditorClient& create_content_browser();
        EditorClient& create_properties_window();
        EditorClient& create_scene_tree();

        ViewportClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;
        ViewportClient& destroy_script_object(ScriptObject* object) override;


        EditorClient& init_world();
        EditorClient& create_log_window(float dt);
        EditorClient& create_viewport_window(float dt);
        void render_dock_window();

        void on_object_select(Object* object);
    };
}// namespace Engine
