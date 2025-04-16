#include <Clients/material/material_client.hpp>
#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/localization.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/string_functions.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/settings.hpp>
#include <Engine/world.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_surface_pool.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader_compiler.hpp>
#include <Widgets/content_browser.hpp>
#include <Widgets/property_renderer.hpp>
#include <Window/window.hpp>
#include <imgui_internal.h>

namespace Engine
{
	trinex_implement_engine_class(MaterialEditorClient, 0)
	{
		register_client(Material::static_class_instance(), static_class_instance());
	}

	class ImGuiMaterialPreview : public ImGuiWidget
	{
		static constexpr float s_default_zooms[] = {1.0f,     1.056962, 1.113924, 1.170886, 1.262025, 1.455696,
													1.740506, 2.025316, 2.310126, 2.594936, 3.164556, 3.734177,
													4.303797, 5.443037, 6.582278, 7.721518, 8.860759, 10.0};

		Pointer<CameraComponent> m_camera;
		Pointer<StaticMeshActor> m_actor;

		World* m_world = nullptr;
		Renderer<EditorSceneRenderer> m_renderer;

		float m_target_zoom  = 1.f;
		float m_current_zoom = 1.f;

		Vector3f m_target_location  = {0, 0.707107, 0.707107};
		Vector3f m_current_location = {0, 0.707107, 0.707107};

		float m_lerp_speed        = 7.f;
		float m_mouse_sensitivity = 1.f;

		int match_zoom_index(int direction)
		{
			int bestIndex      = -1;
			float bestDistance = 0.0f;

			for (int i = 0; i < 18; ++i)
			{
				auto distance = glm::abs(s_default_zooms[i] - m_target_zoom);
				if (distance < bestDistance || bestIndex < 0)
				{
					bestDistance = distance;
					bestIndex    = i;
				}
			}

			if (bestDistance > 0.001f)
			{
				if (direction > 0)
				{
					if (bestIndex < 17)
						++bestIndex;
				}
				else if (direction < 0)
				{
					if (bestIndex > 0)
						--bestIndex;
				}
			}

			return bestIndex;
		}

		float match_zoom(int steps, float fallback)
		{
			auto current_zoom_index = match_zoom_index(steps);
			if (current_zoom_index < 0)
				return fallback;

			auto current_zoom = s_default_zooms[current_zoom_index];
			if (glm::abs(current_zoom - m_target_zoom) > 0.001f)
				return current_zoom;

			auto new_index = current_zoom_index + steps;
			if (new_index >= 0 && new_index < 18)
				return s_default_zooms[new_index];
			else
				return fallback;
		}

	public:
		void init(RenderViewport* viewport)
		{
			m_world          = World::system_of<World>();
			m_renderer.scene = m_world->scene();

			m_camera                  = Object::new_instance<CameraComponent>();
			m_camera->near_clip_plane = 0.1;
			m_actor                   = m_world->spawn_actor<StaticMeshActor>();

			m_actor->mesh_component()->mesh = DefaultResources::Meshes::cube;
			m_actor->mesh_component()->on_transform_changed();
		}

		ImGuiMaterialPreview& material(Material* material)
		{
			m_actor->mesh_component()->material_overrides.resize(1);
			m_actor->mesh_component()->material_overrides[0] = material;
			return *this;
		}

		RenderSurface* render_preview(ImVec2 size, RenderViewport* vp)
		{
			if (size.x < 1.f || size.y < 1.f)
				return nullptr;

			auto pool    = RenderSurfacePool::global_instance();
			auto surface = pool->request_transient_surface(ColorFormat::R8G8B8A8, {size.x, size.y});

			if (surface)
			{
				m_camera->aspect_ratio = size.x / size.y;

				render_thread()->call([this, surface, size, vp, camera_view = m_camera->camera_view()]() {
					SceneView scene_view(camera_view, {size.x, size.y});
					m_renderer.render(scene_view, vp);

					auto dst = surface->rhi_render_target_view();
					auto src = m_renderer.output_surface()->rhi_render_target_view();

					Rect2D rect;
					rect.pos  = {0.f, 0.f};
					rect.size = {size.x, size.y};
					dst->blit(src, rect, rect, SamplerFilter::Point);
				});
			}

			return surface;
		}

		ImGuiMaterialPreview& update_zoom()
		{
			auto& io       = ImGui::GetIO();
			int steps      = ImGui::IsWindowHovered() ? -static_cast<int>(io.MouseWheel) : 0;
			float fallback = s_default_zooms[steps < 0 ? 0 : 17];
			m_target_zoom  = match_zoom(steps, fallback);
			m_current_zoom = glm::mix(m_current_zoom, m_target_zoom, engine_instance->delta_time() * m_lerp_speed);
			return *this;
		}

		ImGuiMaterialPreview& update_rotation(const ImVec2& size)
		{
			if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				const ImVec2 mouse_move = (ImGui::GetIO().MouseDelta / size) * m_mouse_sensitivity;

				float azimuth   = glm::atan(m_target_location.z, m_target_location.x) + mouse_move.x;
				float elevation = glm::acos(m_target_location.y) - mouse_move.y;

				static constexpr float clamp_angle = glm::radians(1.f);

				if (elevation < clamp_angle && elevation > -clamp_angle)
					elevation = elevation < 0.f ? -clamp_angle : clamp_angle;

				m_target_location = glm::normalize(Vector3f{
						glm::sin(elevation) * glm::cos(azimuth),
						glm::cos(elevation),
						glm::sin(elevation) * glm::sin(azimuth),
				});
			}

			m_current_location = glm::mix(m_current_location, m_target_location, engine_instance->delta_time() * m_lerp_speed);
			return *this;
		}

		ImGuiMaterialPreview& update_input(const ImVec2& size)
		{
			update_rotation(size).update_zoom();

			auto bounds        = m_actor->mesh_component()->bounding_box().size();
			float min_distance = glm::max(bounds.x, glm::max(bounds.y, bounds.z));
			min_distance       = glm::length(Vector2f(min_distance, min_distance)) + m_camera->near_clip_plane + 0.01;

			m_camera->location(m_current_location * min_distance * m_current_zoom);
			m_camera->look_at({0.f, 0.f, 0.f});
			return *this;
		}

		bool render(RenderViewport* viewport)
		{
			bool is_open = true;
			ImGui::Begin(name(), &is_open);

			const ImVec2 content_size = ImGui::GetContentRegionAvail();

			update_input(content_size);

			if (auto surface = render_preview(content_size, viewport))
			{
				ImGui::Image(ImTextureID{surface, nullptr}, content_size);
			}

			ImGui::End();
			return is_open;
		}

		~ImGuiMaterialPreview()
		{
			render_thread()->wait();
			//m_world->shutdown();
		}

		static const char* name() { return static_name(); }
		static const char* static_name() { return "editor/MaterialPreview"_localized; }
	};

	class ImGuiMaterialCode : public ImGuiWidget
	{
	public:
		String code;

		void init(RenderViewport* viewport) {}

		bool render(RenderViewport* viewport)
		{
			bool is_open = true;
			ImGui::SetNextWindowSize({300, 400}, ImGuiCond_Appearing);
			ImGui::Begin(name(), &is_open);
			ImGui::InputTextMultiline("##Code", code.data(), code.size(), ImGui::GetContentRegionAvail(),
			                          ImGuiInputTextFlags_ReadOnly);
			ImGui::End();
			return is_open;
		}

		virtual const char* name() const { return static_name(); }

		static const char* static_name() { return "editor/Material Code"_localized; }
	};

	MaterialEditorClient::MaterialEditorClient()
	{
		menu_bar.create("editor/View")->actions.push([this]() {
			draw_available_clients_for_opening();

			if (ImGui::BeginMenu("editor/Material Editor"))
			{
				if (ImGui::MenuItem("editor/Open Content Browser"_localized, nullptr, false, m_content_browser == nullptr))
				{
					create_content_browser();
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
		});

		menu_bar.create("editor/Material")->actions.push([this]() {
			if (ImGui::MenuItem("Compile source", nullptr, false, m_material != nullptr))
			{
				if (auto compiler = ShaderCompiler::instance())
				{
					compiler->compile(m_material);
				}
				else
				{
					error_log("MaterialEditor", "Failed to get material compiler");
				}
			}

			if (ImGui::MenuItem("Just apply", nullptr, false, m_material != nullptr))
			{
				m_material->apply_changes();
			}
		});
	}

	MaterialEditorClient& MaterialEditorClient::create_content_browser()
	{
		m_content_browser = imgui_window()->widgets_list.create<ContentBrowser>();
		m_content_browser->on_close.push([this]() { m_content_browser = nullptr; });
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_preview_window()
	{
		m_preview_window = imgui_window()->widgets_list.create<ImGuiMaterialPreview>();
		m_preview_window->on_close.push([this]() { m_preview_window = nullptr; });

		if (m_material)
			m_preview_window->material(m_material);
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::create_properties_window()
	{
		m_properties_window = imgui_window()->widgets_list.create<PropertyRenderer>();
		m_properties_window->on_close.push([this]() { m_properties_window = nullptr; });

		if (m_material)
			m_properties_window->object(m_material);
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::on_bind_viewport(class RenderViewport* viewport)
	{
		Super::on_bind_viewport(viewport);

		auto wd = window();
		wd->title(Strings::format("Trinex Material Editor [{} RHI]", rhi->info.name.c_str()));

		ImGuiWindow* prev_window = ImGuiWindow::current();

		ImGuiWindow::make_current(imgui_window());
		create_content_browser().create_preview_window().create_properties_window();
		ImGuiWindow::make_current(prev_window);
		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::select(Object* object)
	{
		Super::select(object);

		if (m_material == object)
			return *this;

		if (Material* material = object->instance_cast<Material>())
		{
			m_material = material;

			if (m_properties_window)
			{
				m_properties_window->object(material);
			}

			if (m_preview_window)
			{
				m_preview_window->material(material);
			}
		}

		return *this;
	}

	MaterialEditorClient& MaterialEditorClient::update(float dt)
	{
		Super::update(dt);
		return *this;
	}

	uint32_t MaterialEditorClient::build_dock(uint32_t dock)
	{
		auto properties = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Left, 0.3f, nullptr, &dock);
		auto preview    = ImGui::DockBuilderSplitNode(properties, ImGuiDir_Up, 0.3f, nullptr, &properties);
		auto content    = ImGui::DockBuilderSplitNode(dock, ImGuiDir_Down, 0.3f, nullptr, &dock);

		ImGui::DockBuilderDockWindow(ContentBrowser::static_name(), content);
		ImGui::DockBuilderDockWindow(ImGuiMaterialPreview::static_name(), preview);
		ImGui::DockBuilderDockWindow(PropertyRenderer::static_name(), properties);
		ImGui::DockBuilderDockWindow("###Material Source", dock);
		return dock;
	}
}// namespace Engine
