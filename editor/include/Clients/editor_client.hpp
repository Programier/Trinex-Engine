#pragma once
#include <Core/enum.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene.hpp>
#include <Event/listener_id.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/properties_window.hpp>
#include <editor_scene_renderer.hpp>

namespace Engine
{
    struct EditorState {
        struct {
            const Enum::Entry* view_mode_entry = nullptr;
            Vector2D size                      = {0.f, 0.f};
            bool show_additional_menu          = false;
            bool is_hovered                    = false;
            bool is_using_guizmo               = false;
        } viewport;

        EditorState();
    };

    class EditorClient : public ViewportClient
    {

        declare_class(EditorClient, ViewportClient);

    private:
        class World* m_world = nullptr;
        EditorSceneRenderer m_renderer;
        RenderStatistics m_statistics;
        Flags<ShowFlags, BitMask> m_show_flags;
        SceneView m_scene_view;
        Vector<EventSystemListenerID> m_event_system_listeners;

        class ContentBrowser* m_content_browser;
        ImGuiObjectProperties* m_properties;

        RenderViewport* m_render_viewport = nullptr;
        Window* m_window                  = nullptr;
        size_t m_frame                    = 0;

        class SceneComponent* m_selected_scene_component = nullptr;


        Pointer<CameraComponent> camera;
        float m_camera_speed     = 10.f;
        Vector3D m_camera_move   = {0, 0, 0};
        int_t m_guizmo_operation = 0;
        EditorState m_state;

    public:
        EditorClient();

        // Window manipulation
        void on_content_browser_close();
        void on_properties_window_close();
        EditorClient& create_content_browser();
        EditorClient& create_properties_window();

        ViewportClient& on_bind_viewport(class RenderViewport* viewport) override;
        ViewportClient& on_unbind_viewport(class RenderViewport* viewport) override;
        ViewportClient& render(class RenderViewport* viewport) override;
        ViewportClient& update(class RenderViewport* viewport, float dt) override;

        EditorClient& render_viewport_window(float dt);
        EditorClient& render_guizmo(float dt);
        EditorClient& render_viewport_menu();
        EditorClient& render_dock_window(float dt);
        EditorClient& render_statistics(float dt);

        void on_object_select(Object* object);
        EditorClient& on_object_dropped(Object* object);
        EditorClient& update_drag_and_drop();

        ~EditorClient();

        EditorClient& update_camera(float dt);
        EditorClient& raycast_objects(const Vector2D& coords);

        // Inputs
        void on_mouse_press(const Event& event);
        void on_mouse_release(const Event& event);
        void on_mouse_move(const Event& event);
        void on_finger_move(const Event& event);
        void unbind_window(bool destroying);
        void on_window_close(const Event& event);
    };
}// namespace Engine
