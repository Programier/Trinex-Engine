#pragma once
#include <Engine/camera_types.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>
#include <Event/listener_id.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Widgets/imgui_windows.hpp>

namespace Engine
{
    class EditorClient : public ViewportClient
    {

        declare_class(EditorClient, ViewportClient);

    private:
        SceneRenderer m_renderer;
        CameraView m_view;

        RenderViewport* m_render_viewport = nullptr;
        Window* m_window                  = nullptr;
        size_t m_frame                    = 0;

        class ContentBrowser* m_content_browser;
        ImGuiObjectProperties* m_properties;
        ImGuiSceneTree* m_scene_tree;

        Vector<EventSystemListenerID> m_event_system_listeners;

        class World* m_world                                    = nullptr;
        class GlobalShaderParameters* m_global_shader_params    = nullptr;
        class GlobalShaderParameters* m_global_shader_params_rt = nullptr;
        Vector2D m_viewport_size;
        bool m_viewport_is_hovered = false;

        class CameraComponent* camera;
        float m_camera_speed      = 10.f;
        Vector3D m_camera_move    = {0, 0, 0};
        Index m_target_view_index = 0;

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


        EditorClient& init_world();
        EditorClient& create_log_window(float dt);
        EditorClient& create_viewport_window(float dt);
        void render_dock_window(float dt);

        void on_object_select(Object* object);
        EditorClient& on_object_dropped(Object* object);
        EditorClient& update_drag_and_drop();

        ~EditorClient();

        // Inputs
        void on_mouse_press(const Event& event);
        void on_mouse_release(const Event& event);
        void on_mouse_move(const Event& event);
        void on_key_press(const Event& event);
        void on_key_release(const Event& event);
    };
}// namespace Engine
