#pragma once
#include <Core/pointer.hpp>
#include <Engine/Render/scene_view.hpp>
#include <Engine/Render/scene_view_state.hpp>
#include <Engine/enums.hpp>
#include <UI/controllers/canvas.hpp>

namespace Trinex::UI
{
	class RMLEditor : public RMLClient
	{
		trinex_class(RMLEditor, RMLClient);

	private:
		RML::ElementDocument* m_document = nullptr;
		Identifier m_rml_watch_id        = 0;
		Pointer<class World> m_world;
		Pointer<class CameraComponent> m_camera;
		SceneView m_scene_view;
		SceneViewState m_scene_view_state;
		ShowFlags m_show_flags      = ShowFlags::DefaultFlags;
		ViewMode m_view_mode        = ViewMode::Lit;
		float m_camera_speed        = 10.f;
		Vector3f m_camera_move      = {0.f, 0.f, 0.f};
		bool m_camera_relative_mode = false;

		struct CanvasState {
			Vector2f position = {0.f, 0.f};
			Vector2f size     = {0.f, 0.f};
			Vector2u pixels   = {0, 0};
			bool is_hovered   = false;
		} m_canvas;

	public:
		RMLEditor& attach(class RenderViewport* viewport) override;
		RMLEditor& deattach(class RenderViewport* viewport) override;
		RMLEditor& update(class RenderViewport* viewport, float dt) override;
		EventDispatchResult on_window_event(WindowEvent& event) override;
		EventDispatchResult on_pointer_event(PointerEvent& event) override;
		RMLCanvasFrame render(RML::Element* viewport, const RMLCanvasRenderArgs& args);

	private:
		RMLEditor& update_camera(float dt);
		bool is_pointer_over_canvas(const Vector2f& position) const;
		Vector2f canvas_uv(const Vector2f& position) const;
		void release_relative_camera_mode();
		void on_mouse_press(const PointerEvent& event);
		void on_mouse_release(const PointerEvent& event);
		void on_mouse_move(const PointerEvent& event);
		void select_actors(const Vector2f& uv);
	};
}// namespace Trinex::UI
