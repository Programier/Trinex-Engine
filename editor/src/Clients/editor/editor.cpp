#include <Clients/editor_client.hpp>
#include <Clients/open_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/localization.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/ray.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Engine/world.hpp>
#include <Event/event_data.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Graphics/shader_parameters.hpp>
#include <ImGuizmo.h>
#include <Importer/importer.hpp>
#include <ScriptEngine/script_module.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/imgui_windows.hpp>
#include <Window/window.hpp>
#include <editor_resources.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <icons.hpp>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>
#include <theme.hpp>

namespace Engine
{
	EditorState::EditorState()
	{
		viewport.view_mode_entry =
				Enum::static_find("Engine::ViewMode", true)->entry(static_cast<EnumerateType>(ViewMode::Lit));
	}

	implement_engine_class_default_init(EditorClient, 0);

	EditorClient::EditorClient() : m_show_flags(ShowFlags::DefaultFlags)
	{}

	void EditorClient::unbind_window(bool destroying)
	{
		EventSystem* event_system = EventSystem::instance();

		if (event_system)
		{
			for (auto listener : m_event_system_listeners)
			{
				event_system->remove_listener(listener);
			}
			m_event_system_listeners.clear();
		}

		m_state.window.window = nullptr;
	}

	void EditorClient::on_window_close(const Event& event)
	{
		if (event.window_id() == m_state.window.window->id())
		{
			unbind_window(false);
		}
	}

	EditorClient::~EditorClient()
	{
		unbind_window(true);
	}

	void EditorClient::on_content_browser_close()
	{
		m_content_browser = nullptr;
	}

	void EditorClient::on_properties_window_close()
	{
		m_properties = nullptr;
	}


	EditorClient& EditorClient::create_content_browser()
	{
		m_content_browser = ImGuiRenderer::Window::current()->window_list.create<ContentBrowser>();
		m_content_browser->on_close.push(std::bind(&EditorClient::on_content_browser_close, this));
		m_content_browser->on_object_select.push(
				std::bind(&EditorClient::on_object_select, this, std::placeholders::_1));
		return *this;
	}

	EditorClient& EditorClient::create_properties_window()
	{
		m_properties = ImGuiRenderer::Window::current()->window_list.create<ImGuiObjectProperties>();
		m_properties->on_close.push(std::bind(&EditorClient::on_properties_window_close, this));

		if (m_content_browser)
		{
			on_object_select(m_content_browser->selected_object);
		}
		return *this;
	}


	ViewportClient& EditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Window* window = viewport->window();
		if (window == nullptr)
		{
			throw EngineException("Cannot bind client to non-window viewport!");
		}
		window->imgui_initialize(initialize_theme);

		m_state.window.window		   = window;
		m_state.window.render_viewport = viewport;
		m_state.window.imgui_window	   = window->imgui_window();

		String new_title = Strings::format("Trinex Editor [{} RHI]", rhi->info.name.c_str());
		window->title(new_title);
		render_thread()->wait_all();
		EventSystem::new_system<EventSystem>()->process_event_method(EventSystem::PoolEvents);

		ImGuiRenderer::Window* prev_window = ImGuiRenderer::Window::current();
		ImGuiRenderer::Window::make_current(m_state.window.imgui_window);

		create_content_browser();
		create_properties_window();

		ImGuiRenderer::Window::make_current(prev_window);

		camera = Object::new_instance<CameraComponent>();
		camera->location({0, 10, 10});
		camera->near_clip_plane = 0.1;
		camera->far_clip_plane	= 1000.f;

		EventSystem* event_system = EventSystem::new_system<EventSystem>();
		m_event_system_listeners.push_back(event_system->add_listener(
				EventType::MouseMotion, std::bind(&EditorClient::on_mouse_move, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
				EventType::FingerMotion, std::bind(&EditorClient::on_finger_move, this, std::placeholders::_1)));

		m_event_system_listeners.push_back(event_system->add_listener(
				EventType::MouseButtonDown, std::bind(&EditorClient::on_mouse_press, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
				EventType::MouseButtonUp, std::bind(&EditorClient::on_mouse_release, this, std::placeholders::_1)));
		m_event_system_listeners.push_back(event_system->add_listener(
				EventType::WindowClose, std::bind(&EditorClient::on_window_close, this, std::placeholders::_1)));


		m_world			 = World::new_system<World>();
		m_renderer.scene = m_world->scene();
		m_world->start_play();
		return *this;
	}

	ViewportClient& EditorClient::on_unbind_viewport(class RenderViewport* viewport)
	{
		auto& list = m_state.window.imgui_window->window_list;
		list.close_all_windows();

		unbind_window(false);
		return *this;
	}

	void EditorClient::on_object_select(Object* object)
	{
		if (m_properties)
		{
			m_properties->update(object);
		}

		m_selected_scene_component = Object::instance_cast<SceneComponent>(object);
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

	ViewportClient& EditorClient::render(class RenderViewport* render_viewport)
	{
		ViewPort viewport = rhi->viewport();
		viewport.pos	  = {0.f, 0.f};
		viewport.size	  = SceneRenderTargets::instance()->size() * Settings::e_screen_percentage;

		Scissor scissor;
		scissor.pos	 = {0.f, 0.f};
		scissor.size = viewport.size;

		m_scene_view.viewport(viewport);
		m_scene_view.scissor(scissor);

		m_renderer.render(m_scene_view, render_viewport);

		render_viewport->rhi_bind();
		render_viewport->window()->imgui_window()->rhi_render();

		if (m_scene_view.show_flags() & ShowFlags::Statistics)
		{
			call_in_logic_thread([this, statistics = m_renderer.statistics]() { m_statistics = statistics; });
		}
		return *this;
	}


	EditorClient& EditorClient::render_dock_window(float dt)
	{
		auto dock_id					   = ImGui::GetID("EditorDock##Dock");
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		ImGui::DockSpace(dock_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("editor/View"_localized))
			{
				if (ImGui::MenuItem("editor/Open Material Editor"_localized))
				{
					open_material_editor();
				}

				if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false,
									m_content_browser == nullptr))
				{
					create_content_browser();
				}

				if (ImGui::MenuItem("editor/Open Properties Window"_localized, nullptr, false, m_properties == nullptr))
				{
					create_properties_window();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Edit"_localized))
			{
				if (ImGui::MenuItem("editor/Reload localization"_localized))
				{
					Localization::instance()->reload();
				}

				if (ImGui::BeginMenu("editor/Change language"_localized))
				{
					for (const String& lang : Settings::e_languages)
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

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("editor/Import"_localized))
			{
				bool enable_import = m_content_browser && m_content_browser->selected_package() != nullptr;
				if (ImGui::MenuItem("editor/Import resource"_localized,
									"editor/Import resource from file to selected package"_localized, false,
									enable_import))
				{
					Package* package = m_content_browser->selected_package();
					ImGuiRenderer::Window::current()->window_list.create<ImGuiOpenFile>()->on_select.push(
							[package](const Path& path) { Importer::import_resource(package, path); });
				}

				ImGui::EndMenu();
			}

			ImGui::Text("FPS: %f\n", 1.f / dt);
			ImGui::EndMenuBar();
		}

		if (m_state.window.imgui_window->frame_index() == 1)
		{
			ImGui::DockBuilderRemoveNode(dock_id);
			ImGui::DockBuilderAddNode(dock_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

			auto dock_id_down  = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Down, 0.25f, nullptr, &dock_id);
			auto dock_id_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.25f, nullptr, &dock_id);

			ImGui::DockBuilderDockWindow(ContentBrowser::name(), dock_id_down);
			ImGui::DockBuilderDockWindow(ImGuiObjectProperties::name(), dock_id_right);
			ImGui::DockBuilderDockWindow(Object::localize("editor/Viewport Title").c_str(), dock_id);

			ImGui::DockBuilderFinish(dock_id);
		}

		return *this;
	}

	EditorClient& EditorClient::render_statistics(float dt)
	{
		ImGui::BeginVertical(1, ImGui::GetContentRegionAvail() - ImVec2(100, 0.f), 1.f);
		{
			static const ImVec4 color = {0.f, 1.f, 0.f, 1.f};
			ImGui::TextColored(color, "Delta Time: %f", dt);
			ImGui::TextColored(color, "FPS: %f", 1.f / dt);
			ImGui::TextColored(color, "Visible objects: %zu", m_statistics.visible_objects);
		}
		ImGui::EndVertical();
		return *this;
	}

	ViewportClient& EditorClient::update(class RenderViewport* viewport, float dt)
	{
		ImGuiRenderer::Window* window = m_state.window.imgui_window;
		window->new_frame();

		ImGuiViewport* imgui_viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(imgui_viewport->WorkPos);
		ImGui::SetNextWindowSize(imgui_viewport->WorkSize);
		ImGui::Begin("EditorDock", nullptr,
					 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
							 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar |
							 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar);
		render_dock_window(dt);

		update_camera(dt);
		render_viewport_window(dt);

		ImGui::End();
		window->end_frame();


		if (KeyboardSystem::instance()->is_just_released(Keyboard::Key::Delete))
		{
			auto& selected = m_world->selected_actors();
			for (auto& ell : selected)
			{
				m_world->destroy_actor(ell);
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

		auto view		= camera->view_matrix();
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

	static void render_show_flag(Flags<ShowFlags, BitMask>& flags, ShowFlags flag, const char* name)
	{
		int i_flags = static_cast<int>(flags);
		if (ImGui::CheckboxFlags(name, &i_flags, static_cast<int>(flag)))
		{
			flags.clear_all();
			flags.set(static_cast<BitMask>(i_flags));
		}
	}

	EditorClient& EditorClient::render_viewport_menu()
	{
		const float height			 = 24.f * editor_scale_factor();
		ImVec2 screen_pos			 = ImGui::GetCursorScreenPos();
		static auto render_separator = []() {
			ImU32 color = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
			ImGui::PushStyleColor(ImGuiCol_Separator, color);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.f);
			ImGui::PopStyleColor();

			ImGui::SameLine();
		};

		if (Texture2D* icon = Icons::icon(Icons::More))
		{
			if (ImGui::ImageButton({icon, EditorResources::default_sampler}, {height, height}, {0, 1}, {1, 0}))
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
			if (Texture2D* imgui_texture = Icons::icon(control.second))
			{
				ImVec4 color = control.first == m_guizmo_operation ? ImVec4(0, 0.5f, 0, 1.f) : ImVec4(0, 0, 0, 0);

				if (ImGui::ImageButton({imgui_texture, EditorResources::default_sampler}, {height, height}, {0, 1},
									   {1, 0}, -1, color))
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
			if (ImGui::ImageButton({add_icon, EditorResources::default_sampler}, {height, height}))
			{
				ImGuiRenderer::Window::current()->window_list.create_identified<ImGuiSpawnNewActor>(this, m_world);
			}
		}

		{
			ImGui::SameLine();
			render_separator();
			const auto& transfom = camera->local_transform();
			Vector3D location	 = transfom.location();
			Vector3D rotation	 = transfom.rotation();

			ImGui::BeginGroup();
			ImGui::PushItemWidth(height * 10.f);
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
					static Enum* self = Enum::static_find("Engine::ViewMode", true);

					if (ImGui::BeginCombo("editor/View Mode"_localized, m_state.viewport.view_mode_entry->name.c_str()))
					{
						for (auto& entry : self->entries())
						{
							if (ImGui::Selectable(entry.name.c_str(), &entry == m_state.viewport.view_mode_entry))
							{
								m_state.viewport.view_mode_entry = &entry;
								m_renderer.view_mode(static_cast<ViewMode>(entry.value));
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
						ImGui::EndMenu();
					}
				}

				ImGui::EndPopup();
			}
		}

		return *this;
	}

	EditorClient& EditorClient::render_viewport_window(float dt)
	{
		if (!ImGui::Begin(Object::localize("editor/Viewport Title").c_str(), nullptr))
		{
			ImGui::End();
			return *this;
		};

		if (!m_state.viewport.is_using_guizmo && m_state.viewport.is_hovered &&
			ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			auto relative_mouse_pos = ImGui::GetMousePos() - (ImGui::GetWindowPos() + ImGui::GetCursorPos());
			relative_mouse_pos.y	= m_state.viewport.size.y - relative_mouse_pos.y;
			raycast_objects(ImGuiHelpers::construct_vec2<Vector2D>(relative_mouse_pos));
		}

		{
			auto current_pos = ImGui::GetCursorPos();
			auto size		 = ImGui::GetContentRegionAvail();

			m_state.viewport.size = ImGuiHelpers::construct_vec2<Vector2D>(size);
			camera->aspect_ratio  = m_state.viewport.size.x / m_state.viewport.size.y;

			//auto factor = (m_window->cached_size() * Settings::e_screen_percentage) / m_renderer.output_surface()->size();

			ImGui::Image(reinterpret_cast<Texture2D*>(m_renderer.output_surface()), size,
						 {0.f, Settings::e_screen_percentage}, {Settings::e_screen_percentage, 0.f});
			m_state.viewport.is_hovered = ImGui::IsWindowHovered();

			ImGui::SetCursorPos(current_pos);
			render_guizmo(dt);

			update_drag_and_drop();

			ImGui::SetCursorPos(current_pos);
			ImGui::BeginGroup();
			render_viewport_menu();
			ImGui::EndGroup();

			if (m_show_flags & ShowFlags::Statistics)
			{
				ImGui::SetCursorPos(current_pos + ImVec2(0, ImGui::GetItemRectSize().y));
				render_statistics(dt);
			}
		}

		ImGui::End();
		return *this;
	}

	// Inputs
	static FORCE_INLINE float calculate_y_rotatation(float offset, float size, float fov)
	{
		return -offset * fov / size;
	}

	void EditorClient::on_mouse_press(const Event& event)
	{
		const MouseButtonEvent& button = event.get<const MouseButtonEvent&>();

		if (m_state.viewport.is_hovered && button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(true, m_state.window.window);
		}
	}

	void EditorClient::on_mouse_release(const Event& event)
	{
		const MouseButtonEvent& button = event.get<const MouseButtonEvent&>();

		if (button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(false, m_state.window.window);
			m_camera_move = {0, 0, 0};
		}
	}

	void EditorClient::on_mouse_move(const Event& event)
	{
		const MouseMotionEvent& motion = event.get<const MouseMotionEvent&>();

		if (MouseSystem::instance()->is_relative_mode(m_state.window.window))
		{
			camera->add_rotation(
					{-calculate_y_rotatation(static_cast<float>(motion.yrel), m_state.viewport.size.y, camera->fov),
					 calculate_y_rotatation(static_cast<float>(motion.xrel), m_state.viewport.size.x, camera->fov),
					 0.f});
		}
	}

	void EditorClient::on_finger_move(const Event& event)
	{
		const FingerMotionEvent& motion = event.get<const FingerMotionEvent&>();
		if (motion.finger_index == 0 && m_state.viewport.is_hovered)
		{
			camera->add_rotation(
					{-calculate_y_rotatation(static_cast<float>(motion.yrel), m_state.viewport.size.y, camera->fov),
					 calculate_y_rotatation(static_cast<float>(motion.xrel), m_state.viewport.size.x, camera->fov),
					 0.f});
		}
	}

	static FORCE_INLINE void move_camera(Vector3D& move, Window* window)
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
		move_camera(m_camera_move, m_state.window.window);

		camera->add_location(Vector3D((camera->world_transform().rotation_matrix() * Vector4D(m_camera_move, 1.0))) *
							 dt * m_camera_speed);
		camera->on_transform_changed();

		struct UpdateView : ExecutableObject {
			CameraView view;
			SceneView& out;
			const Flags<ShowFlags, BitMask>& show_flags;

			UpdateView(const CameraView& in, SceneView& out, const Flags<ShowFlags, BitMask>& show_flags)
				: view(in), out(out), show_flags(show_flags)
			{}

			int_t execute() override
			{
				out.camera_view(view);
				out.show_flags(show_flags);
				return sizeof(UpdateView);
			}
		};

		render_thread()->insert_new_task<UpdateView>(camera->camera_view(), m_scene_view, m_show_flags);
		return *this;
	}


	using RaycastPrimitiveResult = Pair<SceneComponent*, float>;

	template<typename NodeType>
	static RaycastPrimitiveResult raycast_primitive(NodeType* node, const Ray& ray,
													RaycastPrimitiveResult result = {nullptr, -1.f})
	{
		if (node == nullptr)
			return result;

		auto intersect = node->box().intersect(ray);

		if (intersect.x > intersect.y)
			return result;

		if (result.first && intersect.x > result.second)
			return result;

		for (auto* component : node->values)
		{
			if (component->component_flags(ActorComponent::DisableRaycast))
				continue;

			intersect = component->bounding_box().intersect(ray);

			if (intersect.x < intersect.y)
			{
				if ((result.first == nullptr) || (intersect.x < result.second))
				{
					result.first  = component;
					result.second = intersect.y;
				}
			}
		}

		for (byte index = 0; index < 8; ++index)
		{
			result = raycast_primitive(node->child_at(index), ray, result);
		}

		return result;
	}

	EditorClient& EditorClient::raycast_objects(const Vector2D& coords)
	{
		SceneView view(camera->camera_view(), m_state.viewport.size);
		Vector3D origin;
		Vector3D direction;

		view.screen_to_world(coords, origin, direction);
		Ray ray(origin, direction);

		// Raycast primitives and lights
		auto component = raycast_primitive(m_world->scene()->primitive_octree().root_node(), ray);
		component	   = raycast_primitive(m_world->scene()->light_octree().root_node(), ray, component);

		if (component.first)
		{
			if (Actor* actor = component.first->actor())
			{
				m_world->unselect_actors();
				m_world->select_actor(actor);
				on_object_select(actor);
			}
		}
		else
		{
			m_world->unselect_actors();
		}

		return *this;
	}
}// namespace Engine
