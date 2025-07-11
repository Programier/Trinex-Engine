#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/etl/average.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene.hpp>
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
			Vector2f size                            = {0.f, 0.f};
			bool show_additional_menu                = false;
			bool is_hovered                          = false;
			bool is_using_guizmo                     = false;
		} viewport;

		EditorState();
	};

	struct RHIPipelineStatistics;
	struct RHITimestamp;

	class EditorClient : public ImGuiViewportClient
	{
		trinex_declare_class(EditorClient, ImGuiViewportClient);

	private:
		struct Stats {
			struct TimingQuery {
				RHITimestamp* timestamp = nullptr;
				const char* pass        = nullptr;
			};

			struct TimingResult {
				float time       = 0.f;
				const char* pass = nullptr;
			};

			struct {
				uint64_t vertices                                = 0;
				uint64_t primitives                              = 0;
				uint64_t geometry_shader_primitives              = 0;
				uint64_t clipping_primitives                     = 0;
				uint64_t vertex_shader_invocations               = 0;
				uint64_t tessellation_control_shader_invocations = 0;
				uint64_t tesselation_shader_invocations          = 0;
				uint64_t geometry_shader_invocations             = 0;
				uint64_t clipping_invocations                    = 0;
				uint64_t fragment_shader_invocations             = 0;

				Vector<RHIPipelineStatistics*> pool;
			} pipeline;

			class
			{
			private:
				CriticalSection m_cs;
				Vector<TimingQuery> m_frames[5];
				Vector<TimingResult> m_frame;
				uint_t m_write_index = 0;
				uint_t m_read_index  = 0;

			public:
				inline Vector<TimingResult>& lock()
				{
					m_cs.lock();
					return m_frame;
				}

				inline void unlock() { m_cs.unlock(); }
				inline Vector<TimingQuery>& writing_frame() { return m_frames[m_write_index]; }
				inline Vector<TimingQuery>& reading_frame() { return m_frames[m_read_index]; }

				inline void submit_read_index() { m_read_index = (m_read_index + 1) % 5u; }
				inline void submit_write_index() { m_write_index = (m_write_index + 1) % 5u; }
			} timings;
		} m_stats;

	private:
		Pointer<World> m_world;

		Vector<Identifier> m_event_system_listeners;
		Vector<class Actor*> m_selected_actors_render_thread;

		Identifier m_on_actor_select_callback_id   = 0;
		Identifier m_on_actor_unselect_callback_id = 0;

		ShowFlags m_show_flags = ShowFlags::DefaultFlags;
		ViewMode m_view_mode   = ViewMode::Lit;
		SceneView m_scene_view;

		class ContentBrowser* m_content_browser = nullptr;
		PropertyRenderer* m_properties          = nullptr;
		ImGuiLevelExplorer* m_level_explorer    = nullptr;

		Pointer<CameraComponent> camera;
		float m_camera_speed     = 10.f;
		Vector3f m_camera_move   = {0, 0, 0};
		int_t m_guizmo_operation = 0;
		EditorState m_state;
		Average<float, 10> m_average_fps = Average<float, 10>(60.f, 1);

		void on_actor_select(World* world, class Actor* actor);
		void on_actor_unselect(World* world, class Actor* actor);
		RenderSurface* capture_scene();

		EditorClient& update_render_stats(Renderer* renderer);

	public:
		EditorClient();

		EditorClient& create_content_browser();
		EditorClient& create_properties_window();
		EditorClient& create_level_explorer();

		EditorClient& on_bind_viewport(class RenderViewport* viewport) override;
		EditorClient& on_unbind_viewport(class RenderViewport* viewport) override;
		EditorClient& update(float dt) override;

		uint32_t build_dock(uint32_t dock_id) override;
		EditorClient& render_viewport_window(float dt);
		EditorClient& render_guizmo(float dt);
		EditorClient& render_viewport_menu();
		EditorClient& render_statistics(float dt);

		EditorClient& on_object_dropped(Object* object);
		EditorClient& update_drag_and_drop();

		EditorClient& update_camera(float dt);
		EditorClient& select_actors(const Vector2f& uv);

		// Inputs
		void on_mouse_press(const Event& event);
		void on_mouse_release(const Event& event);
		void on_mouse_move(const Event& event);
		void on_finger_move(const Event& event);
	};
}// namespace Engine
