#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/etl/average.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene.hpp>
#include <Event/listener_id.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>

namespace Engine
{
	struct EditorState {
		struct {
			const Refl::Enum::Entry* view_mode_entry = nullptr;
			Vector2D size                            = {0.f, 0.f};
			bool show_additional_menu                = false;
			bool is_hovered                          = false;
			bool is_using_guizmo                     = false;
		} viewport;

		EditorState();
	};

	class EditorClient : public ImGuiEditorClient
	{
		declare_class(EditorClient, ImGuiEditorClient);

	private:
		class World* m_world = nullptr;

		Vector<EventSystemListenerID> m_event_system_listeners;
		Identifier m_on_actor_select_callback_id   = 0;
		Identifier m_on_actor_unselect_callback_id = 0;

		EditorSceneRenderer m_renderer;
		RenderStatistics m_statistics;
		ShowFlags m_show_flags = ShowFlags::DefaultFlags;
		SceneView m_scene_view;

		class ContentBrowser* m_content_browser = nullptr;
		PropertyRenderer* m_properties          = nullptr;
		ImGuiLevelExplorer* m_level_explorer    = nullptr;

		Pointer<CameraComponent> camera;
		float m_camera_speed     = 10.f;
		Vector3D m_camera_move   = {0, 0, 0};
		int_t m_guizmo_operation = 0;
		EditorState m_state;
		Average<float, 200> m_average_fps;

		void on_actor_select(World* world, class Actor* actor);
		void on_actor_unselect(World* world, class Actor* actor);

	public:
		// Window manipulation
		EditorClient& create_content_browser();
		EditorClient& create_properties_window();
		EditorClient& create_level_explorer();

		EditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		EditorClient& on_unbind_viewport(class RenderViewport* viewport) override;
		EditorClient& render(class RenderViewport* viewport) override;
		EditorClient& update(float dt) override;

		EditorClient& render_viewport_window(float dt);
		EditorClient& render_guizmo(float dt);
		EditorClient& render_viewport_menu();
		EditorClient& render_dock_window(float dt);
		EditorClient& render_statistics(float dt);

		EditorClient& on_object_dropped(Object* object);
		EditorClient& update_drag_and_drop();

		EditorClient& update_camera(float dt);
		EditorClient& raycast_objects(const Vector2D& coords);

		// Inputs
		void on_mouse_press(const Event& event);
		void on_mouse_release(const Event& event);
		void on_mouse_move(const Event& event);
		void on_finger_move(const Event& event);
	};
}// namespace Engine
