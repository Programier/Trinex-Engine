#include <Core/filesystem/root_filesystem.hpp>
#include <Core/math/math.hpp>
#include <Core/reflection/class.hpp>
#include <Editor/engine.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/Render/render_graph.hpp>
#include <Engine/world.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Input/input_system.hpp>
#include <RHI/context.hpp>
#include <RmlUi/Core.h>
#include <UI/clients/editor.hpp>
#include <Window/window_manager.hpp>

namespace Trinex::UI
{
	namespace
	{
		static void move_camera(Vector3f& move, bool relative_mode)
		{
			move = {0.f, 0.f, 0.f};

			if (!relative_mode)
				return;

			InputSystem* input = InputSystem::instance();
			move.z += input->is_scan_code_pressed_for_user(ScanCode::W) ? 1.f : 0.f;
			move.x += input->is_scan_code_pressed_for_user(ScanCode::D) ? 1.f : 0.f;
			move.z += input->is_scan_code_pressed_for_user(ScanCode::S) ? -1.f : 0.f;
			move.x += input->is_scan_code_pressed_for_user(ScanCode::A) ? -1.f : 0.f;
		}

		static float calculate_y_rotation(float offset, float size)
		{
			return size > 0.f ? Math::radians(-offset * 75.f / size) : 0.f;
		}

		static void make_rotation_quat(float yaw, float pitch, Quaternion& out)
		{
			Quaternion q_pitch = Math::angle_axis(pitch, Vector3f(1, 0, 0));
			Quaternion q_yaw   = Math::angle_axis(yaw, Vector3f(0, 1, 0));
			out                = q_yaw * out * q_pitch;
		}
	}// namespace

	trinex_implement_class(Trinex::UI::RMLEditor, 0) {}

	RMLEditor& RMLEditor::attach(class RenderViewport* viewport)
	{
		Super::attach(viewport);

		m_world = Object::new_instance<World>("World");
		m_world->start_play();

		m_scene_view.state(&m_scene_view_state);
		m_scene_view.scene(m_world->scene());

		m_camera = Object::new_instance<CameraComponent>();
		m_camera->location({0.f, 3.f, -3.f});
		m_camera->look_at({0.f, 0.f, 0.f});

		if ((m_document = context()->LoadDocument("[rml]:/TrinexEditor/layouts/editor/main.rml")))
		{
			m_document->Show();
		}

		m_rml_watch_id = rootfs()->watch("[rml]:/TrinexEditor", [this](const VFS::FileWatchEvent& event) {
			(void) event;

			if (m_document)
				m_document->Close();

			if ((m_document = context()->LoadDocument("[rml]:/TrinexEditor/layouts/editor/main.rml")))
				m_document->Show();
		});

		return *this;
	}

	RMLEditor& RMLEditor::deattach(class RenderViewport* viewport)
	{
		release_relative_camera_mode();

		if (m_rml_watch_id)
		{
			rootfs()->unwatch(m_rml_watch_id);
			m_rml_watch_id = 0;
		}

		if (m_document)
		{
			m_document->Close();
			m_document = nullptr;
		}

		if (m_world)
		{
			m_world->stop_play();
			m_world = nullptr;
		}

		m_camera = nullptr;
		m_canvas = {};
		Super::deattach(viewport);
		return *this;
	}

	RMLEditor& RMLEditor::update(class RenderViewport* viewport, float dt)
	{
		if (m_world)
		{
			m_world->update(dt);
		}

		update_camera(dt);
		Super::update(viewport, dt);
		return *this;
	}

	EventDispatchResult RMLEditor::on_window_event(WindowEvent& event)
	{
		if (event.kind == WindowEventKind::FocusLost)
		{
			release_relative_camera_mode();
			m_canvas.is_hovered = false;
		}

		return Super::on_window_event(event);
	}

	EventDispatchResult RMLEditor::on_pointer_event(PointerEvent& event)
	{
		if (viewport())
		{
			const Vector2f position = {
			        event.screen_position.x,
			        static_cast<float>(viewport()->size().y) - event.screen_position.y,
			};

			m_canvas.is_hovered = is_pointer_over_canvas(position);

			switch (event.kind)
			{
				case PointerEventKind::Moved:
				case PointerEventKind::Entered: on_mouse_move(event); break;
				case PointerEventKind::ButtonPressed: on_mouse_press(event); break;
				case PointerEventKind::ButtonReleased: on_mouse_release(event); break;
				case PointerEventKind::Left:
				{
					m_canvas.is_hovered = false;
					break;
				}
				default: break;
			}
		}

		return Super::on_pointer_event(event);
	}

	RMLCanvasFrame RMLEditor::render(RML::Element* viewport, const RMLCanvasRenderArgs& args)
	{
		(void) viewport;

		m_canvas.position = args.position;
		m_canvas.size     = args.size;

		if (m_world == nullptr || m_camera == nullptr || args.size.x < 1.f || args.size.y < 1.f)
			return {};

		m_scene_view.view_size(args.size);

		EditorRenderer renderer(m_scene_view, m_view_mode);

		const auto& selected = EditorEngine::instance()->selected_actors();
		renderer.render_graph()->add_output(renderer.scene_color_ldr_target());
		renderer.render_grid();
		renderer.render_outlines(selected.data(), selected.size());
		renderer.render_primitives(selected.data(), selected.size());
		renderer.render(RMLEngine::render_context());
		return {.texture = renderer.scene_color_ldr_target()};
	}

	RMLEditor& RMLEditor::update_camera(float dt)
	{
		if (m_camera == nullptr)
			return *this;

		move_camera(m_camera_move, m_camera_relative_mode);

		m_camera->add_location(Vector3f((m_camera->world_transform().rotation_matrix() * Vector4f(m_camera_move, 1.f))) * dt *
		                       m_camera_speed);

		if (m_canvas.size.x > 0.f && m_canvas.size.y > 0.f)
		{
			const float aspect = m_canvas.size.x / m_canvas.size.y;
			m_scene_view.camera_view(m_camera->camera_view(aspect));
			m_scene_view.show_flags(m_show_flags);
		}

		return *this;
	}

	bool RMLEditor::is_pointer_over_canvas(const Vector2f& position) const
	{
		if (m_canvas.size.x <= 0.f || m_canvas.size.y <= 0.f)
			return false;

		return position.x >= m_canvas.position.x && position.y >= m_canvas.position.y &&
		       position.x <= m_canvas.position.x + m_canvas.size.x && position.y <= m_canvas.position.y + m_canvas.size.y;
	}

	Vector2f RMLEditor::canvas_uv(const Vector2f& position) const
	{
		if (!is_pointer_over_canvas(position) || m_canvas.size.x <= 0.f || m_canvas.size.y <= 0.f)
			return {-1.f, -1.f};

		return (position - m_canvas.position) / m_canvas.size;
	}

	void RMLEditor::release_relative_camera_mode()
	{
		if (m_camera_relative_mode)
		{
			m_camera_relative_mode = false;
			WindowManager::instance()->mouse_relative_mode(false);
			m_camera_move = {0.f, 0.f, 0.f};
		}
	}

	void RMLEditor::on_mouse_press(const PointerEvent& event)
	{
		const MouseButton::Enum button = static_cast<MouseButton::Enum>(event.button);

		if (button == MouseButton::Right && m_canvas.is_hovered)
		{
			m_camera_relative_mode = true;
			WindowManager::instance()->mouse_relative_mode(true);
		}
	}

	void RMLEditor::on_mouse_release(const PointerEvent& event)
	{
		const MouseButton::Enum button = static_cast<MouseButton::Enum>(event.button);

		if (button == MouseButton::Right)
		{
			release_relative_camera_mode();
			return;
		}

		if (button == MouseButton::Left && m_canvas.is_hovered)
		{
			const Vector2f position = {
			        event.screen_position.x,
			        static_cast<float>(viewport()->size().y) - event.screen_position.y,
			};

			const Vector2f uv = canvas_uv(position);
			if (uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f)
			{
				select_actors(uv);
			}
		}
	}

	void RMLEditor::on_mouse_move(const PointerEvent& event)
	{
		if (m_camera_relative_mode && m_camera)
		{
			const float pitch = calculate_y_rotation(event.delta.y, m_canvas.size.y);
			const float yaw   = -calculate_y_rotation(event.delta.x, m_canvas.size.x);

			Quaternion rotation = m_camera->local_transform().rotation;
			make_rotation_quat(yaw, pitch, rotation);
			m_camera->rotation(rotation);
		}
	}

	void RMLEditor::select_actors(const Vector2f& uv)
	{
		if (m_world == nullptr)
			return;

		Actor* actor = EditorRenderer::static_raycast(m_scene_view, uv, m_world->scene());

		auto editor = EditorEngine::instance();
		editor->unselect(m_world);

		if (actor)
			editor->select(actor);
	}
}// namespace Trinex::UI
