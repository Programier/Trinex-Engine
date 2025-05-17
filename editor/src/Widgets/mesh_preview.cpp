#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/logger.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/static_mesh_actor.hpp>
#include <Engine/Render/renderer.hpp>
#include <Engine/world.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Widgets/mesh_preview.hpp>

namespace Engine
{
	static constexpr float s_default_zooms[] = {1.0f,     1.056962, 1.113924, 1.170886, 1.262025, 1.455696,
	                                            1.740506, 2.025316, 2.310126, 2.594936, 3.164556, 3.734177,
	                                            4.303797, 5.443037, 6.582278, 7.721518, 8.860759, 10.0};

	static uint find_nearest_size(uint size)
	{
		static constexpr uint s_sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096};
		static constexpr size_t s_count = sizeof(s_sizes) / sizeof(s_sizes[0]);

		auto it = std::lower_bound(s_sizes, s_sizes + s_count, size);
		if (it == std::end(s_sizes))
			return s_sizes[s_count - 1];
		return *it;
	}

	ImGuiStaticMeshPreview::ImGuiStaticMeshPreview()
	{
		m_world                   = World::system_of<World>();
		m_camera                  = Object::new_instance<CameraComponent>();
		m_camera->near_clip_plane = 0.1;
		m_actor                   = m_world->spawn_actor<StaticMeshActor>();

		m_actor->mesh_component()->mesh(DefaultResources::Meshes::cube);
		m_actor->mesh_component()->on_transform_changed();
	}

	int_t ImGuiStaticMeshPreview::match_zoom_index(int direction)
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

	float ImGuiStaticMeshPreview::match_zoom(int steps, float fallback)
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

	ImGuiStaticMeshPreview& ImGuiStaticMeshPreview::material(MaterialInterface* material)
	{
		m_actor->mesh_component()->material_overrides.resize(1);
		m_actor->mesh_component()->material_overrides[0] = material;
		return *this;
	}

	RenderSurface* ImGuiStaticMeshPreview::render_preview(ImVec2 size)
	{
		if (size.x < 1.f || size.y < 1.f)
			return nullptr;

		auto pool          = RenderSurfacePool::global_instance();
		Vector2u view_size = {find_nearest_size(size.x), find_nearest_size(size.y)};
		auto surface       = pool->request_transient_surface(SurfaceFormat::RGBA8, view_size);

		if (surface)
		{
			m_camera->aspect_ratio = size.x / size.y;

			render_thread()->call([this, surface, view_size, camera_view = m_camera->camera_view()]() {
				SceneView scene_view(camera_view, view_size);
				Renderer* renderer = Renderer::static_create_renderer(m_world->scene(), scene_view);
				EditorRenderer::render_grid(renderer);

				renderer->render();

				auto dst = surface->rhi_rtv();
				auto src = renderer->scene_color_target()->as_rtv();

				Rect2D rect;
				rect.pos  = {0.f, 0.f};
				rect.size = view_size;
				dst->blit(src, rect, rect, SamplerFilter::Bilinear);
			});
		}

		return surface;
	}

	ImGuiStaticMeshPreview& ImGuiStaticMeshPreview::update_zoom()
	{
		auto& io       = ImGui::GetIO();
		int steps      = ImGui::IsWindowHovered() ? -static_cast<int>(io.MouseWheel) : 0;
		float fallback = s_default_zooms[steps < 0 ? 0 : 17];
		m_target_zoom  = match_zoom(steps, fallback);
		m_current_zoom = glm::mix(m_current_zoom, m_target_zoom, engine_instance->delta_time() * m_lerp_speed);
		return *this;
	}

	ImGuiStaticMeshPreview& ImGuiStaticMeshPreview::update_rotation(const ImVec2& size)
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

	ImGuiStaticMeshPreview& ImGuiStaticMeshPreview::update_input(const ImVec2& size)
	{
		update_rotation(size).update_zoom();

		auto bounds        = m_actor->mesh_component()->bounding_box().size();
		float min_distance = glm::max(bounds.x, glm::max(bounds.y, bounds.z));
		min_distance       = glm::length(Vector2f(min_distance, min_distance)) + m_camera->near_clip_plane + 0.01;

		m_camera->location(m_current_location * min_distance * m_current_zoom);
		m_camera->look_at({0.f, 0.f, 0.f});
		return *this;
	}

	bool ImGuiStaticMeshPreview::render(RenderViewport*)
	{
		bool is_open = true;
		ImGui::Begin(name(), &is_open);

		const ImVec2 content_size = ImGui::GetContentRegionAvail();

		update_input(content_size);

		if (auto surface = render_preview(content_size))
		{
			ImGui::Image(surface, content_size);
		}

		ImGui::End();
		return is_open;
	}

	const char* ImGuiStaticMeshPreview::name()
	{
		return static_name();
	}

	const char* ImGuiStaticMeshPreview::static_name()
	{
		return "editor/StaticMeshPreview"_localized;
	}

}// namespace Engine
