#include <Core/base_engine.hpp>
#include <Core/colors.hpp>
#include <Core/default_resources.hpp>
#include <Core/editor_resources.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/batched_primitives.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Graphics/editor_scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
	extern void render_editor_grid(const CameraView& view);
	static void render_light_sprite(Texture2D* texture, LightComponent* component, const SceneView& view)
	{
		Material* material                 = DefaultResources::Materials::sprite;
		PositionVertexBuffer* vertex_bufer = DefaultResources::Buffers::screen_position;

		if (auto* parameter = material->find_parameter<MaterialParameters::Float4x4>(Name::model))
		{
			Transform transform = component->proxy()->world_transform();
			transform.scale({0.5, 0.5, 0.5});
			transform.look_at(view.camera_view().location, Constants::OY);
			parameter->value = transform.matrix();
		}

		auto* texture_parameter = material->find_parameter<MaterialParameters::Sampler2D>(Name::texture);
		Texture2D* tmp_texture  = nullptr;
		Sampler* tmp_sampler    = nullptr;

		if (texture_parameter && texture)
		{
			tmp_texture                = texture_parameter->texture;
			tmp_sampler                = texture_parameter->sampler;
			texture_parameter->texture = texture;
			texture_parameter->sampler = EditorResources::default_sampler;
		}

		material->apply(component);
		vertex_bufer->rhi_bind(0, 0);
		rhi->draw(6, 0);

		if (texture_parameter && texture)
		{
			texture_parameter->texture = tmp_texture;
			texture_parameter->sampler = tmp_sampler;
		}
	}

	static void render_spot_light_overlay_colored(SpotLightComponent* component, float angle, Vector4D color)
	{
		auto proxy         = component->proxy();
		Material* material = EditorResources::spot_light_overlay_material;

		auto model_param = material->find_parameter<MaterialParameters::Float4x4>(Name::model);
		auto transform   = proxy->world_transform();
		transform.scale({1, 1, 1});
		model_param->value = transform.matrix();

		auto radius_param = material->find_parameter<MaterialParameters::Float>(Name::radius);
		auto height_param = material->find_parameter<MaterialParameters::Float>(Name::height);

		static constexpr float sphere_radius = 4.f;
		float radius                         = glm::sin(angle) * sphere_radius;
		float height                         = glm::cos(angle) * sphere_radius;

		radius_param->value = radius;
		height_param->value = height;

		auto color_param   = material->find_parameter<MaterialParameters::Float4>(Name::color);
		color_param->value = color;

		material->apply();

		EditorResources::spot_light_overlay_positions->rhi_bind(0);
		rhi->draw(EditorResources::spot_light_overlay_positions->buffer.size(), 0);
	}

	static void render_spot_light_overlay(SpotLightComponent* component)
	{
		auto proxy = component->proxy();
		render_spot_light_overlay_colored(component, proxy->outer_cone_angle(), {1.0, 1.0, 1.0, 1.0});
		render_spot_light_overlay_colored(component, proxy->inner_cone_angle(), {0.7, 0.7, 0.7, 1.0});
	}

	static void render_point_light_overlay(PointLightComponent* component)
	{
		auto proxy         = component->proxy();
		Material* material = EditorResources::point_light_overlay_material;

		auto color_parameter  = material->find_parameter<MaterialParameters::Float4>(Name::color);
		auto offset_parameter = material->find_parameter<MaterialParameters::Float3>(Name::offset);
		auto radius_parameter = material->find_parameter<MaterialParameters::Float>(Name::radius);

		if (color_parameter)
		{
			color_parameter->value = Colors::White;
		}

		if (offset_parameter)
		{
			offset_parameter->value = proxy->world_transform().location();
		}

		if (radius_parameter)
		{
			radius_parameter->value = proxy->attenuation_radius();
		}

		material->apply();
		EditorResources::point_light_overlay_positions->rhi_bind(0, 0);
		rhi->draw(EditorResources::point_light_overlay_positions->buffer.size(), 0);
	}

	class OverlaySceneLayer : public SceneLayer
	{
	public:
		BatchedLines lines;
		BatchedTriangles triangles;

		Set<LightComponent*> m_light_components;

		OverlaySceneLayer& clear() override
		{
			SceneLayer::clear();
			m_light_components.clear();
			triangles.clear();
			lines.clear();
			return *this;
		}

		OverlaySceneLayer& render(SceneRenderer* renderer, RenderViewport* rt) override
		{
			SceneRenderTargets::instance()->begin_rendering_scene_color_ldr();

			triangles.render(renderer->scene_view());
			lines.render(renderer->scene_view());

			for (LightComponent* component : m_light_components)
			{
				render_light_sprite(EditorResources::light_sprite, component, renderer->scene_view());

				if (component->actor()->is_selected())
				{
					if (component->leaf_class_is<SpotLightComponent>())
					{
						render_spot_light_overlay(reinterpret_cast<SpotLightComponent*>(component));
					}
					else if (component->leaf_class_is<PointLightComponent>())
					{
						render_point_light_overlay(reinterpret_cast<PointLightComponent*>(component));
					}
				}
			}

			render_editor_grid(renderer->scene_view().camera_view());

			SceneRenderTargets::instance()->end_rendering_scene_color_ldr();
			return *this;
		}
	};

	static void create_directional_arrow(DirectionalLightComponent* component, OverlaySceneLayer* layer)
	{
		DirectionalLightComponentProxy* proxy = component->proxy();
		auto& transform                       = proxy->world_transform();
		auto location                         = transform.location();
		auto direction                        = proxy->direction();

		constexpr float offset        = 0.5f;
		const Vector3D forward_vector = transform.forward_vector();
		const Vector3D right_vector   = transform.right_vector();

		Vector3D end_point        = location + direction * 3.f;
		Vector3D arrow_base_point = end_point - direction * offset;


		static const ByteColor white = {255, 150, 150, 255};
		static const ByteColor red   = {255, 0, 0, 255};

		Vector3D arrow_points[4] = {
		        arrow_base_point + forward_vector * offset / 2.f,
		        arrow_base_point + right_vector * offset / 2.f,
		        arrow_base_point + forward_vector * -offset / 2.f,
		        arrow_base_point + right_vector * -offset / 2.f,
		};

		layer->lines.add_line(location, end_point);
		auto& triangles = layer->triangles;

		triangles.add_triangle(arrow_points[0], end_point, arrow_points[1], white, red, white);
		triangles.add_triangle(arrow_points[1], end_point, arrow_points[2], white, red, white);
		triangles.add_triangle(arrow_points[2], end_point, arrow_points[3], white, red, white);
		triangles.add_triangle(arrow_points[3], end_point, arrow_points[0], white, red, white);
		triangles.add_triangle(arrow_points[0], arrow_points[1], arrow_points[2], white, white, white);
		triangles.add_triangle(arrow_points[2], arrow_points[3], arrow_points[0], white, white, white);
	}


	EditorSceneRenderer::EditorSceneRenderer()
	{
		m_overlay_layer = post_process_layer()->create_next<OverlaySceneLayer>("Overlay Layer");
	}

	EditorSceneRenderer& EditorSceneRenderer::render_component(LightComponent* component)
	{
		ColorSceneRenderer::render_component(component);
		m_overlay_layer->m_light_components.insert(component);

		if (component->actor()->is_selected())
		{
			if (DirectionalLightComponent* directional_light = component->instance_cast<DirectionalLightComponent>())
			{
				create_directional_arrow(directional_light, m_overlay_layer);
			}
		}

		return *this;
	}

	EditorSceneRenderer& EditorSceneRenderer::render_component(PrimitiveComponent* component)
	{
		ColorSceneRenderer::render_component(component);

		Actor* owner = component->actor();
		if (owner == nullptr)
			return *this;

		if (owner->is_selected() && owner->scene_component() == component)
		{
			component->proxy()->bounding_box().write_to_batcher(m_overlay_layer->lines, {255, 0, 0, 255});
		}

		return *this;
	}
}// namespace Engine
