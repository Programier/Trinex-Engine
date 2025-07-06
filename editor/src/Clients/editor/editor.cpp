#include <Clients/editor_client.hpp>
#include <Core/editor_config.hpp>
#include <Core/editor_resources.hpp>
#include <Core/icons.hpp>
#include <Core/importer.hpp>
#include <Core/localization.hpp>
#include <Core/reflection/class.hpp>
#include <Core/theme.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/ray.hpp>
#include <Engine/settings.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/texture_2D.hpp>
#include <ImGuizmo.h>
#include <Platform/platform.hpp>
#include <RHI/rhi.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace Engine
{
	EditorState::EditorState()
	{
		viewport.view_mode_entry = Refl::Enum::static_find("Engine::ViewMode", Refl::FindFlags::IsRequired)
		                                   ->entry(static_cast<EnumerateType>(ViewMode::Lit));
	}

	trinex_implement_engine_class(EditorClient, 0)
	{
		register_client(Actor::static_class_instance(), static_class_instance());
	}

	EditorClient::EditorClient()
	{
		menu_bar.create("editor/View")->actions.push([this]() {
			draw_available_clients_for_opening();

			if (ImGui::BeginMenu("editor/Editor"_localized))
			{
				if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
				{
					create_content_browser();
				}

				if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, m_properties == nullptr))
				{
					create_properties_window();
				}
				ImGui::EndMenu();
			}
		});

		menu_bar.create("editor/Edit")->actions.push([]() {
			if (ImGui::MenuItem("editor/Reload localization"_localized))
			{
				Localization::instance()->reload();
			}

			if (ImGui::BeginMenu("editor/Change language"_localized))
			{
				for (const String& lang : Settings::languages)
				{
					const char* localized = Object::localize("editor/" + lang).c_str();
					if (ImGui::MenuItem(localized))
					{
						Object::language(lang);
						break;
					}
				}

				ImGui::EndMenu();
			}

			ImGui::SliderFloat("Screen Percentage", &Settings::screen_percentage, 0.f, 2.f);
		});

		menu_bar.create("editor/Import")->actions.push([this]() {
			bool enable_import = m_content_browser && m_content_browser->selected_package() != nullptr;
			if (ImGui::MenuItem("editor/Import resource"_localized,
			                    "editor/Import resource from file to selected package"_localized, false, enable_import))
			{
				Package* package = m_content_browser->selected_package();
				imgui_window()->widgets_list.create<ImGuiOpenFile>()->on_select.push(
				        [package](const Path& path) { Importer::import_resource(package, path); });
			}
		});

		menu_bar.create("")->actions.push([this]() {
			ImGui::Text("FPS: %.2f\n", m_average_fps.average());
			ImGui::Spacing();
			ImGui::Text("RHI: %s\n", rhi->info.name.c_str());
			ImGui::Spacing();
			ImGui::Text("GPU: %s\n", rhi->info.renderer.c_str());
		});
	}

	EditorClient& EditorClient::create_content_browser()
	{
		m_content_browser = imgui_window()->widgets_list.create<ContentBrowser>();
		m_content_browser->on_close.push([this]() { m_content_browser = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::create_properties_window()
	{
		m_properties = imgui_window()->widgets_list.create<PropertyRenderer>();
		m_properties->on_close.push([this]() { m_properties = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::create_level_explorer()
	{
		m_level_explorer = imgui_window()->widgets_list.create<ImGuiLevelExplorer>(m_world);
		m_level_explorer->on_close.push([this]() { m_level_explorer = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);

		auto wd          = window();
		String new_title = Strings::format("Trinex Editor [{} RHI]", rhi->info.name.c_str());
		wd->title(new_title);

		auto monitor_info = Platform::monitor_info(wd->monitor_index());
		wd->size(monitor_info.size);

		EventSystem::system_of<EventSystem>()->process_event_method(EventSystem::PoolEvents);
		m_world                       = World::system_of<World>();
		m_on_actor_select_callback_id = m_world->on_actor_select.push(
		        std::bind(&This::on_actor_select, this, std::placeholders::_1, std::placeholders::_2));
		m_on_actor_unselect_callback_id = m_world->on_actor_unselect.push(
		        std::bind(&This::on_actor_unselect, this, std::placeholders::_1, std::placeholders::_2));
		m_world->start_play();

		ImGuiWindow* prev_window = ImGuiWindow::current();
		ImGuiWindow::make_current(imgui_window());

		create_content_browser();
		create_properties_window();
		create_level_explorer();

		ImGuiWindow::make_current(prev_window);

		camera = Object::new_instance<CameraComponent>();
		camera->location({0, 10, 10});
		camera->near_clip_plane = 0.1f;
		camera->far_clip_plane  = 1000.f;

		EventSystem* event_system = EventSystem::system_of<EventSystem>();
		m_event_system_listeners.push_back(event_system->add_listener(
		        EventType::MouseMotion, std::bind(&EditorClient::on_mouse_move, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
		        EventType::FingerMotion, std::bind(&EditorClient::on_finger_move, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
		        EventType::MouseButtonDown, std::bind(&EditorClient::on_mouse_press, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
		        EventType::MouseButtonUp, std::bind(&EditorClient::on_mouse_release, this, std::placeholders::_1)));
		return *this;
	}

	EditorClient& EditorClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		Super::on_unbind_viewport(viewport);

		m_world->on_actor_select.remove(m_on_actor_select_callback_id);
		m_world->on_actor_unselect.remove(m_on_actor_unselect_callback_id);

		m_world->stop_play();

		EventSystem* event_system = EventSystem::instance();

		if (event_system)
		{
			for (auto listener : m_event_system_listeners)
			{
				event_system->remove_listener(listener);
			}
			m_event_system_listeners.clear();
		}

		return *this;
	}

	void EditorClient::on_actor_select(World* world, class Actor* actor)
	{
		if (m_properties)
		{
			m_properties->object(actor);
		}

		render_thread()->call([this, actor]() { m_selected_actors_render_thread.push_back(actor); });
	}

	void EditorClient::on_actor_unselect(World* world, class Actor* actor)
	{
		if (m_properties)
		{
			if (m_properties->object() == actor || actor == nullptr)
			{
				m_properties->object(nullptr);
			}
		}

		render_thread()->call([this, actor]() {
			auto it = std::find(m_selected_actors_render_thread.begin(), m_selected_actors_render_thread.end(), actor);

			if (it != m_selected_actors_render_thread.end())
				m_selected_actors_render_thread.erase(it);
		});
	}

	RenderSurface* EditorClient::capture_scene()
	{
		Vector2i size        = viewport()->size();
		RenderSurface* scene = RenderSurfacePool::global_instance()->request_transient_surface(RHISurfaceFormat::RGBA8, size);

		if (scene == nullptr)
			return nullptr;

		render_thread()->call([this, scene, mode = m_view_mode, size]() {
			m_scene_view.viewport(RHIViewport(size));
			m_scene_view.scissor(RHIScissors(size));

			Renderer* renderer = Renderer::static_create_renderer(m_world->scene(), m_scene_view, mode);
			EditorRenderer::render_grid(renderer);

			size_t selected_count = m_selected_actors_render_thread.size();

			if (selected_count != 0)
				EditorRenderer::render_outlines(renderer, m_selected_actors_render_thread.data(), selected_count);

			EditorRenderer::render_primitives(renderer, m_selected_actors_render_thread.data(), selected_count);


			RHIRect rect = m_scene_view.viewport().size;
			auto src     = renderer->render().scene_color_target()->as_rtv();
			auto dst     = scene->rhi_rtv();
			dst->blit(src, rect, rect, RHISamplerFilter::Point);

			if ((m_scene_view.show_flags() & ShowFlags::Statistics) != ShowFlags::None)
			{
				call_in_logic_thread([this, statistics = renderer->statistics]() { m_statistics = statistics; });
			}
		});

		return scene;
	}

	EditorClient& EditorClient::update_drag_and_drop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ContentBrowser->Object");
			if (payload)
			{
				IM_ASSERT(payload->DataSize == sizeof(Object*));
				on_object_dropped(*reinterpret_cast<Object**>(payload->Data));
			}
			ImGui::EndDragDropTarget();
		}
		return *this;
	}

	EditorClient& EditorClient::render_statistics(float dt)
	{
		ImGui::BeginVertical(1, ImGui::GetContentRegionAvail() - ImVec2(100, 0.f), 1.f);
		{
			static const ImVec4 color = {0.f, 1.f, 0.f, 1.f};
			ImGui::TextColored(color, "Delta Time: %f", dt);
			ImGui::TextColored(color, "FPS: %f", m_average_fps.average());
			ImGui::TextColored(color, "Visible objects: %zu", m_statistics.visible_objects);
		}
		ImGui::EndVertical();
		return *this;
	}

	EditorClient& EditorClient::update(float dt)
	{
		m_average_fps.push(1.0 / dt);
		update_camera(dt);
		render_viewport_window(dt);

		if (KeyboardSystem::instance()->is_just_released(Keyboard::Key::Delete))
		{
			auto& selected = m_world->selected_actors();
			while (!selected.empty())
			{
				m_world->destroy_actor(*selected.begin());
			}
		}

		return *this;
	}

	EditorClient& EditorClient::render_guizmo(float dt)
	{
		const auto& selected = m_world->selected_actors();

		if (selected.empty())
		{
			m_state.viewport.is_using_guizmo = false;
			return *this;
		}

		ImGuizmo::BeginFrame();
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y, m_state.viewport.size.x,
		                  m_state.viewport.size.y);

		auto view       = camera->view_matrix();
		auto projection = camera->projection_matrix();

		for (Actor* actor : selected)
		{
			SceneComponent* component = actor->scene_component();
			if (!component)
				continue;

			const auto& transform = component->world_transform();
			{
				if (m_guizmo_operation == 0)
				{
					m_guizmo_operation = ImGuizmo::OPERATION::UNIVERSAL;
				}
				Matrix4f model = transform.matrix();
				if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				                         static_cast<ImGuizmo::OPERATION>(m_guizmo_operation), ImGuizmo::MODE::WORLD,
				                         glm::value_ptr(model), nullptr, nullptr))
				{

					Transform new_transform = model;
					new_transform /= component->parent()->world_transform();
					component->local_transform(new_transform);
				}
			}
		}

		m_state.viewport.is_using_guizmo =
		        ImGuizmo::IsOver(static_cast<ImGuizmo::OPERATION>(m_guizmo_operation) | ImGuizmo::OPERATION::SCALE);
		return *this;
	}

	static void render_show_flag(ShowFlags& flags, ShowFlags flag, const char* name)
	{
		ImU64 mask = static_cast<ImU64>(flags);

		if (ImGui::CheckboxFlags(name, &mask, static_cast<ImU64>(flag)))
		{
			flags = static_cast<ShowFlags>(mask);
		}
	}

	EditorClient& EditorClient::render_viewport_menu()
	{
		const float height           = ImGui::GetFontSize();
		ImVec2 screen_pos            = ImGui::GetCursorScreenPos();
		static auto render_separator = []() {
			ImU32 color = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
			ImGui::PushStyleColor(ImGuiCol_Separator, color);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.f);
			ImGui::PopStyleColor();

			ImGui::SameLine();
		};

		if (ImTextureID icon = Icons::icon(Icons::More))
		{
			if (ImGui::ImageButton(icon, {height, height}))
			{
				m_state.viewport.show_additional_menu = true;
				ImGui::OpenPopup("##addition_menu");
				ImGui::SetNextWindowPos(screen_pos + ImVec2(0, height + ImGui::GetStyle().ItemSpacing.y * 2.f),
				                        ImGuiCond_Appearing);
			}

			ImGui::SameLine();
		}

		static const Pair<ImGuizmo::OPERATION, Icons::IconType> controls[] = {
		        {ImGuizmo::OPERATION::UNIVERSAL, Icons::IconType::Select},
		        {ImGuizmo::OPERATION::TRANSLATE, Icons::IconType::Move},
		        {ImGuizmo::OPERATION::ROTATE, Icons::IconType::Rotate},
		        {ImGuizmo::OPERATION::SCALE, Icons::IconType::Scale},
		};

		for (auto& control : controls)
		{
			if (ImTextureID imgui_texture = Icons::icon(control.second))
			{
				ImVec4 color = control.first == m_guizmo_operation ? ImVec4(0, 0.5f, 0, 1.f) : ImVec4(0, 0, 0, 0);

				if (ImGui::ImageButton(imgui_texture, {height, height}, {0, 0}, {1, 1}, color))
				{
					m_guizmo_operation = control.first;
				}

				if (ImGui::IsItemHovered())
				{
					m_state.viewport.is_hovered = false;
				}

				ImGui::SameLine();
			}
		}

		auto add_icon = Icons::icon(Icons::IconType::Add);

		if (add_icon)
		{
			render_separator();
			if (ImGui::ImageButton(add_icon, {height, height}))
			{
				imgui_window()->widgets_list.create_identified<ImGuiSpawnNewActor>(this, m_world);
			}
		}

		{
			ImGui::SameLine();
			render_separator();
			const auto& transfom = camera->local_transform();
			Vector3f location    = transfom.location();
			Vector3f rotation    = transfom.rotation();

			ImGui::BeginGroup();
			ImGui::PushItemWidth(height * 15.f);
			bool changed = ImGui::InputFloat3("editor/Loc"_localized, &location.x);
			ImGui::SameLine();
			changed = ImGui::InputFloat3("editor/Rot"_localized, &rotation.x) || changed;
			ImGui::PopItemWidth();
			ImGui::EndGroup();

			if (changed)
			{
				camera->location(location);
				camera->rotation(rotation);
				camera->on_transform_changed();
			}
		}

		if (m_state.viewport.show_additional_menu)
		{
			if (ImGui::BeginPopup("##addition_menu", ImGuiWindowFlags_NoMove))
			{
				{
					static auto* self = Refl::Enum::static_find("Engine::ViewMode", Refl::FindFlags::IsRequired);

					if (ImGui::BeginCombo("editor/View Mode"_localized, m_state.viewport.view_mode_entry->name.c_str()))
					{
						for (auto& entry : self->entries())
						{
							if (ImGui::Selectable(entry.name.c_str(), &entry == m_state.viewport.view_mode_entry))
							{
								m_state.viewport.view_mode_entry = &entry;
								m_view_mode                      = static_cast<ViewMode::Enum>(entry.value);
							}
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginMenu("editor/Show Flags"_localized))
					{
						render_show_flag(m_show_flags, ShowFlags::Statistics, "Statistics");
						render_show_flag(m_show_flags, ShowFlags::Sprite, "Sprite");
						render_show_flag(m_show_flags, ShowFlags::StaticMesh, "Static Mesh");
						render_show_flag(m_show_flags, ShowFlags::Wireframe, "Wireframe");
						render_show_flag(m_show_flags, ShowFlags::Gizmo, "Gizmo");
						render_show_flag(m_show_flags, ShowFlags::PointLights, "Point Lights");
						render_show_flag(m_show_flags, ShowFlags::SpotLights, "Spot Lights");
						render_show_flag(m_show_flags, ShowFlags::DirectionalLights, "Directional Lights");
						render_show_flag(m_show_flags, ShowFlags::PostProcess, "Post Process");
						render_show_flag(m_show_flags, ShowFlags::LightOctree, "Light Octree");
						render_show_flag(m_show_flags, ShowFlags::PrimitiveOctree, "Primitive Octree");
						ImGui::Checkbox("Show Grid", &Settings::Editor::show_grid);
						ImGui::EndMenu();
					}
				}

				ImGui::EndPopup();
			}
		}

		return *this;
	}

	uint32_t EditorClient::build_dock(uint32_t dock_id)
	{
		auto dock_id_right      = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);
		auto dock_id_right_up   = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.5f, nullptr, &dock_id_right);
		auto dock_id_right_down = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.5f, nullptr, &dock_id_right);
		auto dock_id_down       = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.25f, nullptr, &dock_id);

		ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), dock_id_down);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), dock_id_right_up);
		ImGui::DockBuilderDockWindow(ImGuiLevelExplorer::static_name(), dock_id_right_down);
		ImGui::DockBuilderDockWindow(Object::localize("editor/Viewport").c_str(), dock_id);
		return dock_id;
	}

	EditorClient& EditorClient::render_viewport_window(float dt)
	{
		if (!ImGui::Begin(Object::localize("editor/Viewport").c_str(), nullptr))
		{
			ImGui::End();
			return *this;
		};

		auto cursor_position = ImGui::GetCursorPos();
		auto viewport_size   = ImGui::GetContentRegionAvail();

		if (!m_state.viewport.is_using_guizmo && m_state.viewport.is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			auto relative_mouse_pos = ImGui::GetMousePos() - (ImGui::GetWindowPos() + ImGui::GetCursorPos());
			select_actors(ImGui::EngineVecFrom(relative_mouse_pos / viewport_size));
		}

		{
			m_state.viewport.size = ImGui::EngineVecFrom(viewport_size);
			camera->aspect_ratio  = m_state.viewport.size.x / m_state.viewport.size.y;

			if (RenderSurface* scene = capture_scene())
			{
				ImGui::Image(scene, viewport_size);
				m_state.viewport.is_hovered = ImGui::IsWindowHovered();
			}

			ImGui::SetCursorPos(cursor_position);
			render_guizmo(dt);

			update_drag_and_drop();

			ImGui::SetCursorPos(cursor_position);
			ImGui::BeginGroup();
			render_viewport_menu();
			ImGui::EndGroup();

			if ((m_show_flags & ShowFlags::Statistics) != ShowFlags::None)
			{
				ImGui::SetCursorPos(cursor_position + ImVec2(0, ImGui::GetItemRectSize().y));
				render_statistics(dt);
			}
		}

		ImGui::End();
		return *this;
	}

	static FORCE_INLINE void move_camera(Vector3f& move, Window* window)
	{
		move = {0, 0, 0};

		if (!MouseSystem::instance()->is_relative_mode(window))
			return;
		KeyboardSystem* keyboard = KeyboardSystem::instance();

		move.z += keyboard->is_pressed(Keyboard::W) ? -1.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::A) ? -1.f : 0.f;
		move.z += keyboard->is_pressed(Keyboard::S) ? 1.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::D) ? 1.f : 0.f;
	}

	EditorClient& EditorClient::update_camera(float dt)
	{
		move_camera(m_camera_move, window());

		camera->add_location(Vector3f((camera->world_transform().rotation_matrix() * Vector4f(m_camera_move, 1.0))) * dt *
		                     m_camera_speed);

		struct UpdateView : Task<UpdateView> {
			CameraView view;
			SceneView& out;
			ShowFlags show_flags;

			UpdateView(const CameraView& in, SceneView& out, ShowFlags show_flags) : view(in), out(out), show_flags(show_flags) {}

			void execute() override
			{
				out.camera_view(view);
				out.show_flags(show_flags);
			}
		};

		render_thread()->create_task<UpdateView>(camera->camera_view(), m_scene_view, m_show_flags);
		return *this;
	}


	using RaycastPrimitiveResult = Pair<SceneComponent*, float>;

	EditorClient& EditorClient::select_actors(const Vector2f& uv)
	{
		Actor* actor = nullptr;

		render_thread()->call([this, uv, &actor]() { actor = EditorRenderer::raycast(m_scene_view, uv, m_world->scene()); });
		render_thread()->wait();

		m_world->unselect_actors();

		if (actor)
			m_world->select_actor(actor);
		return *this;
	}

	// Inputs
	static FORCE_INLINE float calculate_y_rotatation(float offset, float size, float fov)
	{
		return -offset * fov / size;
	}

	void EditorClient::on_mouse_press(const Event& event)
	{
		const auto& button = event.mouse.button;

		if (m_state.viewport.is_hovered && button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(true, window());
		}
	}

	void EditorClient::on_mouse_release(const Event& event)
	{
		const auto& button = event.mouse.button;

		if (button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(false, window());
			m_camera_move = {0, 0, 0};
		}
	}

	void EditorClient::on_mouse_move(const Event& event)
	{
		const auto& motion = event.mouse.motion;

		if (MouseSystem::instance()->is_relative_mode(window()))
		{
			camera->add_rotation({-calculate_y_rotatation(static_cast<float>(motion.yrel), m_state.viewport.size.y, camera->fov),
			                      calculate_y_rotatation(static_cast<float>(motion.xrel), m_state.viewport.size.x, camera->fov),
			                      0.f});
		}
	}

	void EditorClient::on_finger_move(const Event& event)
	{
		const auto& motion = event.touchscreen.finger_motion;
		if (motion.index == 0 && m_state.viewport.is_hovered)
		{
			camera->add_rotation({-calculate_y_rotatation(static_cast<float>(motion.yrel), m_state.viewport.size.y, camera->fov),
			                      calculate_y_rotatation(static_cast<float>(motion.xrel), m_state.viewport.size.x, camera->fov),
			                      0.f});
		}
	}
}// namespace Engine
