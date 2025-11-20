#pragma once
#include <Clients/imgui_client.hpp>
#include <Core/etl/average.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/reflection/enum.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/camera_view.hpp>
#include <Engine/scene.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/render_viewport.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Widgets/property_renderer.hpp>

namespace ImGuizmo
{
	enum OPERATION : unsigned int;
	enum MODE : unsigned int;
}// namespace ImGuizmo

namespace Engine
{
	struct EditorState {
		struct {
			const Refl::Enum::Entry* view_mode_entry = nullptr;
			Vector2f size                            = {0.f, 0.f};
			bool show_additional_menu                = false;
			bool is_hovered                          = false;
		} viewport;

		EditorState();
	};

	class RHIPipelineStatistics;
	class RHITimestamp;
	class RHIFence;
	class CameraComponent;

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

			class Pipeline
			{
			private:
				struct Entry {
					RHIPipelineStatistics* stats = nullptr;
					RHIFence* fence              = nullptr;
				};

				Deque<Entry> m_pool;
				RHIPipelineStatistics* m_last = nullptr;

			public:
				void update();
				void submit();

				RHIPipelineStatistics* new_frame();
				inline RHIPipelineStatistics* last_frame() { return m_last; }
				~Pipeline();
			} pipeline;

			class Timings
			{
			private:
				struct Frame {
					RHIFence* fence;
					Vector<TimingQuery> queries;

					void clear();
					~Frame();
				};

				Deque<Frame> m_frames;
				Vector<TimingResult> m_timings;

			public:
				void update();
				void submit();

				Vector<TimingQuery>& new_frame();
				inline const Vector<TimingResult>& last_frame() const { return m_timings; }
			} timings;

			inline void update()
			{
				timings.update();
				pipeline.update();
			}

			inline void submit()
			{
				timings.submit();
				pipeline.submit();
			}
		} m_stats;

	private:
		Pointer<World> m_world;

		Vector<Identifier> m_event_system_listeners;

		Identifier m_on_actor_select_callback_id   = 0;
		Identifier m_on_actor_unselect_callback_id = 0;

		ShowFlags m_show_flags = ShowFlags::DefaultFlags;
		ViewMode m_view_mode   = ViewMode::Lit;
		SceneView m_scene_view;

		class ContentBrowser* m_content_browser = nullptr;
		PropertyRenderer* m_properties          = nullptr;
		ImGuiLevelExplorer* m_level_explorer    = nullptr;

		Pointer<CameraComponent> camera;
		float m_camera_speed   = 10.f;
		Vector3f m_camera_move = {0, 0, 0};

		EditorState m_state;
		Average<float, 10> m_dt;
		ImGuizmo::OPERATION m_guizmo_operation;
		ImGuizmo::MODE m_guizmo_mode;

	private:
		void on_actor_select(World* world, class Actor* actor);
		void on_actor_unselect(World* world, class Actor* actor);
		RHITexture* capture_scene();

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
		EditorClient& render_viewport_window();
		EditorClient& render_guizmo();
		EditorClient& render_viewport_menu();
		EditorClient& render_statistics();

		EditorClient& on_object_dropped(Object* object);
		EditorClient& update_drag_and_drop();

		EditorClient& update_camera();
		EditorClient& select_actors(const Vector2f& uv);

		// Inputs
		void on_mouse_press(const Event& event);
		void on_mouse_release(const Event& event);
		void on_mouse_move(const Event& event);
		void on_finger_move(const Event& event);
	};
}// namespace Engine
