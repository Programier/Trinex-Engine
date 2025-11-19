#define IMVIEWGUIZMO_IMPLEMENTATION 1

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
#include <Engine/Render/render_graph.hpp>
#include <Engine/settings.hpp>
#include <Engine/world.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/texture.hpp>
#include <ImGuizmo.h>
#include <ImViewGuizmo.h>
#include <Platform/platform.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>
#include <Systems/event_system.hpp>
#include <Systems/keyboard_system.hpp>
#include <Systems/mouse_system.hpp>
#include <UI/primitives.hpp>
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
		register_client(Actor::static_reflection(), static_reflection());
	}

	EditorClient::EditorClient()
	{
		m_guizmo_operation = ImGuizmo::OPERATION::UNIVERSAL;
		m_guizmo_mode      = ImGuizmo::MODE::WORLD;

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

		menu_bar.create("editor/Edit")->actions.push([this]() {
			if (ImGui::MenuItem("editor/Reload localization"_localized))
			{
				Localization::instance()->reload();
			}

			if (ImGui::MenuItem("editor/Open Style Editor"_localized))
			{
				window()->widgets.create_identified<ImGuiStyleEditor>("Editor Style Editor");
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
				window()->widgets.create<ImGuiOpenFile>()->on_select.push(
				        [package](const Path& path) { Importer::import_scene(package, path); });
			}

			if (ImGui::MenuItem("editor/Import scene"_localized, "editor/Import scene from file to world"_localized, false,
			                    enable_import))
			{
				Package* package = m_content_browser->selected_package();
				window()->widgets.create<ImGuiOpenFile>()->on_select.push(
				        [package, world = m_world](const Path& path) { Importer::import_scene(package, path, world); });
			}
		});

		menu_bar.create("")->actions.push([this]() {
			ImGui::Text("FPS: %.2f\n", 1.f / m_dt.average());
			ImGui::Spacing();
			ImGui::Text("RHI: %s\n", rhi->info.name.c_str());
			ImGui::Spacing();
			ImGui::Text("GPU: %s\n", rhi->info.renderer.c_str());
		});
	}

	EditorClient& EditorClient::create_content_browser()
	{
		m_content_browser = window()->widgets.create<ContentBrowser>();
		m_content_browser->on_close.push([this]() { m_content_browser = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::create_properties_window()
	{
		m_properties = window()->widgets.create<PropertyRenderer>();
		m_properties->on_close.push([this]() { m_properties = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::create_level_explorer()
	{
		m_level_explorer = window()->widgets.create<ImGuiLevelExplorer>(m_world);
		m_level_explorer->on_close.push([this]() { m_level_explorer = nullptr; });
		return *this;
	}

	EditorClient& EditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);

		auto wd          = window()->window();
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
		ImGuiWindow::make_current(window());

		create_content_browser();
		create_properties_window();
		create_level_explorer();

		ImGuiWindow::make_current(prev_window);

		camera = Object::new_instance<CameraComponent>();
		camera->location({0, 3.f, -3.f});
		camera->look_at({0.f, 0.f, 0.f});

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
	}

	RHITexture* EditorClient::capture_scene()
	{
		Vector2i size = viewport()->size();

		m_scene_view.view_size(size);

		EditorRenderer renderer(m_world->scene(), m_scene_view, m_view_mode);
		update_render_stats(&renderer);

		const auto& selected = m_world->selected_actors();
		renderer.render_graph()->add_output(renderer.scene_color_ldr_target());
		renderer.render_grid();
		renderer.render_outlines(selected);
		renderer.render_primitives(selected);

		RHIContextPool::global_instance()->execute([&](RHIContext* ctx) { renderer.render(ctx); });

		return renderer.scene_color_ldr_target();
	}

	EditorClient& EditorClient::update_render_stats(Renderer* renderer)
	{
		if (!m_stats.pipeline.pool.empty())
		{
			// auto stats = m_stats.pipeline.pool.front();

			// if (stats->is_ready())
			// {
			// 	size_t data[10] = {
			// 	        stats->vertices(),
			// 	        stats->primitives(),
			// 	        stats->geometry_shader_primitives(),
			// 	        stats->clipping_primitives(),
			// 	        stats->vertex_shader_invocations(),
			// 	        stats->tessellation_control_shader_invocations(),
			// 	        stats->tesselation_shader_invocations(),
			// 	        stats->geometry_shader_invocations(),
			// 	        stats->clipping_invocations(),
			// 	        stats->fragment_shader_invocations(),
			// 	};

			// 	call_in_logic_thread([=, this]() {
			// 		m_stats.pipeline.vertices                                = data[0];
			// 		m_stats.pipeline.primitives                              = data[1];
			// 		m_stats.pipeline.geometry_shader_primitives              = data[2];
			// 		m_stats.pipeline.clipping_primitives                     = data[3];
			// 		m_stats.pipeline.vertex_shader_invocations               = data[4];
			// 		m_stats.pipeline.tessellation_control_shader_invocations = data[5];
			// 		m_stats.pipeline.tesselation_shader_invocations          = data[6];
			// 		m_stats.pipeline.geometry_shader_invocations             = data[7];
			// 		m_stats.pipeline.clipping_invocations                    = data[8];
			// 		m_stats.pipeline.fragment_shader_invocations             = data[9];
			// 	});

			// 	RHIPipelineStatisticsPool::global_instance()->return_statistics(stats);
			// 	m_stats.pipeline.pool.erase(m_stats.pipeline.pool.begin());
			// }
		}

		if (!m_stats.timings.reading_frame().empty())
		{
			// auto& frame = m_stats.timings.reading_frame();

			// bool is_ready = true;
			// for (size_t i = 0, count = frame.size(); is_ready && i < count; ++i)
			// {
			// 	is_ready = frame[i].timestamp->is_ready();
			// }

			// if (is_ready)
			// {
			// 	auto& result = m_stats.timings.lock();
			// 	result.clear();

			// 	auto pool = RHITimestampPool::global_instance();

			// 	for (auto& query : frame)
			// 	{
			// 		result.emplace_back(query.timestamp->milliseconds(), query.pass);
			// 		pool->return_timestamp(query.timestamp);
			// 	}

			// 	frame.clear();
			// 	m_stats.timings.unlock();

			// 	m_stats.timings.submit_read_index();
			// }
		}

		if (!(m_scene_view.show_flags() & ShowFlags::Statistics))
			return *this;

		if (m_stats.pipeline.pool.size() < 5)
		{
			class Plugin : public RenderGraph::Graph::Plugin
			{
				RHIPipelineStatistics* m_stats;

			public:
				Plugin(EditorClient* client) : m_stats(RHIPipelineStatisticsPool::global_instance()->request_statistics())
				{
					client->m_stats.pipeline.pool.push_back(m_stats);
				}

				Plugin& on_frame_begin(RenderGraph::Graph* graph, RHIContext* ctx)
				{
					ctx->begin_statistics(m_stats);
					return *this;
				}

				Plugin& on_frame_end(RenderGraph::Graph* graph, RHIContext* ctx)
				{
					ctx->end_statistics(m_stats);
					return *this;
				}
			};

			renderer->render_graph()->create_plugin<Plugin>(this);
		}

		if (m_stats.timings.writing_frame().empty())
		{
			auto& frame = m_stats.timings.writing_frame();

			class Plugin : public RenderGraph::Graph::Plugin
			{
				Vector<Stats::TimingQuery>& m_frame;

			public:
				Plugin(Vector<Stats::TimingQuery>& frame) : m_frame(frame) {}

				Plugin& on_pass_begin(RenderGraph::Pass* pass, RHIContext* ctx)
				{
					Stats::TimingQuery entry;
					entry.pass      = pass->name();
					entry.timestamp = RHITimestampPool::global_instance()->request_timestamp();
					ctx->begin_timestamp(entry.timestamp);

					m_frame.push_back(entry);
					return *this;
				}

				Plugin& on_pass_end(RenderGraph::Pass* pass, RHIContext* ctx)
				{
					Stats::TimingQuery& entry = m_frame.back();
					ctx->end_timestamp(entry.timestamp);
					return *this;
				}
			};

			renderer->render_graph()->create_plugin<Plugin>(frame);
			m_stats.timings.submit_write_index();
		}
		return *this;
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

	EditorClient& EditorClient::render_statistics()
	{
		ImGui::BeginVertical(1, ImGui::GetContentRegionAvail() - ImVec2(100, 0.f), 0.f);
		{
			static const ImVec4 color = {0.f, 1.f, 0.f, 1.f};
			ImGui::TextColored(color, "Delta Time: %f", m_dt.average());
			ImGui::TextColored(color, "FPS: %f", 1.f / m_dt.average());

			ImGui::TextColored(color, "Vertices: %zu", m_stats.pipeline.vertices);
			ImGui::TextColored(color, "Primitives: %zu", m_stats.pipeline.primitives);
			ImGui::TextColored(color, "Clipping primitives: %zu", m_stats.pipeline.clipping_primitives);
			ImGui::TextColored(color, "Geometry primitives: %zu", m_stats.pipeline.geometry_shader_primitives);

			ImGui::TextColored(color, "Vertex shader invocations: %zu", m_stats.pipeline.vertex_shader_invocations);
			ImGui::TextColored(color, "Geometry shader invocations: %zu", m_stats.pipeline.geometry_shader_invocations);
			ImGui::TextColored(color, "Tessellation control shader invocations: %zu",
			                   m_stats.pipeline.tessellation_control_shader_invocations);
			ImGui::TextColored(color, "Tessellation shader invocations: %zu", m_stats.pipeline.tesselation_shader_invocations);
			ImGui::TextColored(color, "Geometry shader invocations: %zu", m_stats.pipeline.geometry_shader_invocations);
			ImGui::TextColored(color, "Clipping invocations: %zu", m_stats.pipeline.clipping_invocations);
			ImGui::TextColored(color, "Fragment shader invocations: %zu", m_stats.pipeline.fragment_shader_invocations);

			ImGui::NewLine();

			auto& timings = m_stats.timings.lock();

			float total = 0.f;
			for (auto& time : timings)
			{
				ImGui::TextColored(color, "%s: %f ms", time.pass, time.time);
				total += time.time;
			}

			ImGui::TextColored(color, "Total: %f", total);
			m_stats.timings.unlock();
		}
		ImGui::EndVertical();
		return *this;
	}

	EditorClient& EditorClient::update(float dt)
	{
		m_dt.push(dt);
		update_camera();
		render_viewport_window();

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

	EditorClient& EditorClient::render_guizmo()
	{
		const ImVec2 cursor       = ImGui::GetCursorScreenPos();
		const ImVec2 content_size = ImGui::GetContentRegionAvail();

		// Render view Guizmo
		{
			ImViewGuizmo::Begin();
			auto& guizmo_style = ImViewGuizmo::GetStyle();

			const ImVec2 padding      = ImGui::GetStyle().FramePadding * ImVec2(-1.f, 1.f);
			const float guizmo_radius = guizmo_style.bigCircleRadius * guizmo_style.scale;
			const float button_radius = guizmo_style.toolButtonRadius * guizmo_style.scale;

			auto loc   = camera->local_transform().location;
			auto pivot = loc;
			auto rot   = camera->local_transform().rotation;
			auto pos   = cursor + ImVec2(content_size.x, 0.f) + ImVec2(-guizmo_radius, guizmo_radius) + padding;

			if (ImViewGuizmo::Rotate(loc, rot, pivot, pos))
			{
				camera->location(loc).rotation(rot);
			}

			pos.x += guizmo_radius - (button_radius * 2.f);
			pos.y += guizmo_radius + button_radius + padding.y;

			if (ImViewGuizmo::Dolly(loc, rot, pos))
			{
				camera->location(loc).rotation(rot);
			}

			pos.y += button_radius * 2.f + padding.y;

			if (ImViewGuizmo::Pan(loc, rot, pos))
			{
				camera->location(loc).rotation(rot);
			}
		}

		const auto& selected = m_world->selected_actors();

		if (selected.size() == 0)
			return *this;

		ImGuizmo::BeginFrame();
		ImGuizmo::AllowAxisFlip(false);
		ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(cursor.x, cursor.y, m_state.viewport.size.x, m_state.viewport.size.y);

		auto view       = camera->view_matrix();
		auto projection = Math::scale(Matrix4f(1.f), {1.f, -1.f, 1.f}) * camera->projection_matrix(viewport()->aspect());

		for (Actor* actor : selected)
		{
			SceneComponent* component = actor->scene_component();
			if (!component)
				continue;

			const auto& transform = component->world_transform();
			{
				Matrix4f model = transform.matrix();
				if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
				                         static_cast<ImGuizmo::OPERATION>(m_guizmo_operation), m_guizmo_mode,
				                         glm::value_ptr(model), nullptr, nullptr))
				{

					Transform new_transform = model;
					new_transform -= component->parent()->world_transform();
					component->local_transform(new_transform);
				}
			}
		}


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

	static bool render_viewport_item_button(const char* id, UI::IconDrawFunc func, bool active = false)
	{
		if (active)
		{
			auto active_color = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
			ImGui::PushStyleColor(ImGuiCol_Button, active_color);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::MakeHoveredColor(active_color));
		}

		const bool result = UI::icon_button(func, id, ImGui::GetFontSize());

		if (active)
			ImGui::PopStyleColor(2);

		return result;
	}

	EditorClient& EditorClient::render_viewport_menu()
	{
		const float height = ImGui::GetFontSize();
		ImVec2 screen_pos  = ImGui::GetCursorScreenPos();

		static auto render_separator = []() {
			ImU32 color = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));
			ImGui::PushStyleColor(ImGuiCol_Separator, color);
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 3.f);
			ImGui::PopStyleColor();

			ImGui::SameLine();
		};

		struct {
			ImGuizmo::OPERATION op;
			UI::IconDrawFunc func;
			const char* name;
		} operation_controls[] = {
		        {ImGuizmo::OPERATION::UNIVERSAL, UI::select_icon, "##op_universal"},
		        {ImGuizmo::OPERATION::TRANSLATE, UI::move_icon, "##op_translate"},
		        {ImGuizmo::OPERATION::ROTATE, UI::rotate_icon, "##op_rotate"},
		        {ImGuizmo::OPERATION::SCALE, UI::scale_icon, "##op_scale"},
		};

		if (UI::icon_button(UI::more_icon, "##View", height))
		{
			m_state.viewport.show_additional_menu = true;
			ImGui::OpenPopup("##addition_menu");
			ImGui::SetNextWindowPos(screen_pos + ImVec2(0, height + ImGui::GetStyle().ItemSpacing.y * 2.f), ImGuiCond_Appearing);
		}

		ImGui::SameLine();

		for (auto& control : operation_controls)
		{
			const bool is_active = m_guizmo_operation == control.op;

			if (render_viewport_item_button(control.name, control.func, is_active))
			{
				m_guizmo_operation = control.op;
			}

			if (ImGui::IsItemHovered())
			{
				m_state.viewport.is_hovered = false;
			}

			ImGui::SameLine();
		}

		if (m_properties)
		{
			if (render_viewport_item_button("##Camera", UI::camera_icon, m_properties->object() == camera))
			{
				m_properties->object(camera);
			}
			ImGui::SameLine();
		}
		{
			if (render_viewport_item_button("###globe_mode", UI::globe_icon, ImGuizmo::WORLD == m_guizmo_mode))
			{
				m_guizmo_mode = m_guizmo_mode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			}

			ImGui::SameLine();
		}

		render_separator();

		if (UI::icon_button(UI::plus_icon, "##SpawnActor", height))
		{
			window()->widgets.create_identified<ImGuiSpawnNewActor>(this, m_world);
		}

		{
			ImGui::SameLine();
			render_separator();
			const auto& transfom = camera->local_transform();
			Vector3f location    = transfom.location;

			ImGui::BeginGroup();
			ImGui::PushItemWidth(height * 15.f);
			bool changed = ImGui::InputFloat3("editor/Loc"_localized, &location.x);
			ImGui::PopItemWidth();
			ImGui::EndGroup();

			if (changed)
			{
				camera->location(location);
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
						for (auto& entry : ShowFlags::static_reflection()->entries())
						{
							render_show_flag(m_show_flags, entry.value, entry.name.c_str());
						}

						ImGui::Checkbox("Show Grid", &Settings::Editor::show_grid);
						ImGui::EndMenu();
					}

					ImGui::DragFloat("Camera Speed", &m_camera_speed, 0.5f, 1.f, 512.f);
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

	EditorClient& EditorClient::render_viewport_window()
	{
		if (!ImGui::Begin(Object::localize("editor/Viewport").c_str(), nullptr))
		{
			ImGui::End();
			return *this;
		};

		auto cursor_position = ImGui::GetCursorPos();
		auto viewport_size   = ImGui::GetContentRegionAvail();

		if (!ImGuizmo::IsOver() && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
		    ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			auto relative_mouse_pos = ImGui::GetMousePos() - (ImGui::GetWindowPos() + ImGui::GetCursorPos());
			Vector2f uv             = ImGui::EngineVecFrom(relative_mouse_pos / viewport_size);

			if (uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f)
				select_actors(uv);
		}

		{
			m_state.viewport.size = ImGui::EngineVecFrom(viewport_size);

			if (RHITexture* scene = capture_scene())
			{
				ImGui::Image(scene, viewport_size);
				m_state.viewport.is_hovered = ImGui::IsWindowHovered();
			}

			ImGui::SetCursorPos(cursor_position);
			render_guizmo();

			update_drag_and_drop();

			ImGui::SetCursorPos(cursor_position);
			ImGui::BeginGroup();
			render_viewport_menu();
			ImGui::EndGroup();

			if ((m_show_flags & ShowFlags::Statistics) != ShowFlags::None)
			{
				float padding = ImGui::GetTextLineHeightWithSpacing();
				ImGui::SetCursorPos(cursor_position + ImVec2(padding, padding + ImGui::GetItemRectSize().y));
				render_statistics();
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

		move.z += keyboard->is_pressed(Keyboard::W) ? 1.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::D) ? 1.f : 0.f;
		move.z += keyboard->is_pressed(Keyboard::S) ? -1.f : 0.f;
		move.x += keyboard->is_pressed(Keyboard::A) ? -1.f : 0.f;
	}

	EditorClient& EditorClient::update_camera()
	{
		move_camera(m_camera_move, window()->window());

		camera->add_location(Vector3f((camera->world_transform().rotation_matrix() * Vector4f(m_camera_move, 1.0))) *
		                     m_dt.average() * m_camera_speed);

		float aspect = m_state.viewport.size.x / m_state.viewport.size.y;

		if (aspect > 0.f)
		{
			m_scene_view.camera_view(camera->camera_view(aspect));
			m_scene_view.prev_camera_view(camera->prev_camera_view(aspect));
			m_scene_view.show_flags(m_show_flags);
		}

		return *this;
	}

	EditorClient& EditorClient::select_actors(const Vector2f& uv)
	{
		Actor* actor = EditorRenderer::static_raycast(m_scene_view, uv, m_world->scene());

		m_world->unselect_actors();

		if (actor)
			m_world->select_actor(actor);
		return *this;
	}

	// Inputs
	void EditorClient::on_mouse_press(const Event& event)
	{
		const auto& button = event.mouse.button;

		if (m_state.viewport.is_hovered && button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(true, window()->window());
		}
	}

	void EditorClient::on_mouse_release(const Event& event)
	{
		const auto& button = event.mouse.button;

		if (button.button == Mouse::Button::Right)
		{
			MouseSystem::instance()->relative_mode(false, window()->window());
			m_camera_move = {0, 0, 0};
		}
	}

	static FORCE_INLINE float calculate_y_rotation(float offset, float size)
	{
		return Math::radians(-offset * 75.f / size);
	}

	static FORCE_INLINE void make_rotation_quat(float yaw, float pitch, Quaternion& out)
	{
		Quaternion q_pitch = Math::angle_axis(pitch, Vector3f(1, 0, 0));
		Quaternion q_yaw   = Math::angle_axis(yaw, Vector3f(0, 1, 0));
		out                = q_yaw * out * q_pitch;
	}

	void EditorClient::on_mouse_move(const Event& event)
	{
		const auto& motion = event.mouse.motion;

		if (MouseSystem::instance()->is_relative_mode(window()->window()))
		{
			float pitch = calculate_y_rotation(static_cast<float>(motion.yrel), m_state.viewport.size.y);
			float yaw   = -calculate_y_rotation(static_cast<float>(motion.xrel), m_state.viewport.size.x);

			Quaternion rotation = camera->local_transform().rotation;
			make_rotation_quat(yaw, pitch, rotation);
			camera->rotation(rotation);
		}
	}

	void EditorClient::on_finger_move(const Event& event)
	{
		const auto& motion = event.touchscreen.finger_motion;

		if (motion.index == 0 && m_state.viewport.is_hovered)
		{
			float pitch = calculate_y_rotation(static_cast<float>(motion.yrel), m_state.viewport.size.y);
			float yaw   = -calculate_y_rotation(static_cast<float>(motion.xrel), m_state.viewport.size.x);

			Quaternion rotation = camera->local_transform().rotation;
			make_rotation_quat(yaw, pitch, rotation);
			camera->rotation(rotation);
		}
	}

}// namespace Engine
