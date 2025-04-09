#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_surface_pool.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	static const AABB_3Df light_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

	trinex_implement_engine_class(LightComponent, 0)
	{
		Refl::Class* self = static_class_instance();

		trinex_refl_prop(self, This, m_light_color, Refl::Property::IsColor)->tooltip("Color of this light");
		trinex_refl_prop(self, This, m_is_enabled)//
				->display_name("Is Enabled")
				.tooltip("Is light enabled");

		trinex_refl_prop(self, This, m_is_shadows_enabled)
				->display_name("Enable Shadows")
				.tooltip("The light source can cast real-time shadows");

		trinex_refl_prop(self, This, m_intensivity)//
				->display_name("Intensivity")
				.tooltip("Intensivity of this light");

		trinex_refl_prop(self, This, m_depth_bias);
		trinex_refl_prop(self, This, m_slope_scale);
	}

	LightComponent::LightComponent()
		: m_light_color({1.0, 1.0, 1.0}), m_intensivity(30.f), m_depth_bias(0.5f), m_slope_scale(0.5f), m_is_enabled(true),
		  m_is_shadows_enabled(false)
	{}

	LightComponent& LightComponent::on_transform_changed()
	{
		Super::on_transform_changed();

		if (Scene* world_scene = scene())
		{
			world_scene->update_light_transform(this);
		}

		return *this;
	}

	LightComponent& LightComponent::start_play()
	{
		Super::start_play();
		Scene* world_scene = scene();
		if (world_scene)
		{
			world_scene->add_light(this);
		}
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::render(SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	ActorComponentProxy* LightComponent::create_proxy()
	{
		return new LightComponentProxy();
	}

	LightComponentProxy* LightComponent::proxy() const
	{
		return typed_proxy<LightComponentProxy>();
	}

	LightComponent& LightComponent::stop_play()
	{
		Super::stop_play();

		Scene* world_scene = scene();

		if (world_scene)
		{
			world_scene->remove_light(this);
		}
		return *this;
	}

	LightComponent& LightComponent::light_color(const Color3& color)
	{
		m_light_color = color;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::intensivity(float value)
	{
		m_intensivity = value;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::is_enabled(bool enabled)
	{
		m_is_enabled = enabled;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::is_shadows_enabled(bool enabled)
	{
		m_is_shadows_enabled = enabled;

		if (enabled && m_shadow_map == nullptr)
		{
			m_shadow_map = RenderSurfacePool::global_instance()->request_surface(ColorFormat::ShadowDepth, {1024, 1024});
		}
		else if (!enabled && m_shadow_map != nullptr)
		{
			RenderSurfacePool::global_instance()->return_surface(m_shadow_map.ptr());
		}

		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::submit_light_info_render_thread()
	{
		render_thread()->call([proxy           = proxy(),             //
							   bounds          = m_bounds,            //
							   color           = m_light_color,       //
							   intensivity     = m_intensivity,       //
							   depth_bias      = m_depth_bias,        //
							   slope_scale     = m_slope_scale,       //
							   enabled         = m_is_enabled,        //
							   shadows_enabled = m_is_shadows_enabled,//
							   map             = m_shadow_map]() {
			proxy->m_bounds             = bounds;
			proxy->m_light_color        = color;
			proxy->m_intensivity        = intensivity;
			proxy->m_depth_bias         = depth_bias;
			proxy->m_slope_scale        = slope_scale;
			proxy->m_is_enabled         = enabled;
			proxy->m_is_shadows_enabled = shadows_enabled;
			proxy->m_shadow_map         = map;
		});
		return *this;
	}

	LightComponent& LightComponent::update_bounding_box()
	{
		m_bounds = AABB_3Df(light_bounds).center(world_transform().location());
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_class_instance())
		{
			if (event.property->address(this) == &m_is_shadows_enabled)
			{
				is_shadows_enabled(m_is_shadows_enabled);
			}
			else
			{
				submit_light_info_render_thread();
			}
		}

		return *this;
	}

	LightComponent::~LightComponent() {}

	SceneRenderer& SceneRenderer::render_component(LightComponent* component)
	{
		return *this;
	}
}// namespace Engine
