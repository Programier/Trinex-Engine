#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/Render/shadow_map_pool.hpp>
#include <Engine/scene.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	static const AABB_3Df light_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

	Name LightComponent::name_color             = "light_data.color";
	Name LightComponent::name_intensivity       = "light_data.intensivity";
	Name LightComponent::name_location          = "light_data.location";
	Name LightComponent::name_radius            = "light_data.radius";
	Name LightComponent::name_fall_off_exponent = "light_data.fall_off_exponent";
	Name LightComponent::name_direction         = "light_data.direction";
	Name LightComponent::name_spot_angles       = "light_data.spot_angles";

	const AABB_3Df& LightComponentProxy::bounding_box() const
	{
		return m_bounds;
	}

	const Color3& LightComponentProxy::light_color() const
	{
		return m_light_color;
	}

	float LightComponentProxy::intensivity() const
	{
		return m_intensivity;
	}

	bool LightComponentProxy::is_enabled() const
	{
		return m_is_enabled;
	}

	bool LightComponentProxy::is_shadows_enabled() const
	{
		return m_is_shadows_enabled;
	}

	LightComponentProxy& LightComponentProxy::bounding_box(const AABB_3Df& bounds)
	{
		m_bounds = bounds;
		return *this;
	}

	LightComponentProxy& LightComponentProxy::light_color(const Color3& color)
	{
		m_light_color = color;
		return *this;
	}

	LightComponentProxy& LightComponentProxy::intensivity(float value)
	{
		m_intensivity = value;
		return *this;
	}

	LightComponentProxy& LightComponentProxy::is_enabled(bool enabled)
	{
		m_is_enabled = enabled;
		return *this;
	}

	LightComponentProxy& LightComponentProxy::is_shadows_enabled(bool enabled)
	{
		m_is_shadows_enabled = enabled;
		return *this;
	}

	LightComponentProxy& LightComponentProxy::shadow_map(RenderSurface* map)
	{
		m_shadow_map = map;
		return *this;
	}

	implement_engine_class(LightComponent, 0)
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
	}

	LightComponent::LightComponent()
	    : m_light_color({1.0, 1.0, 1.0}), m_intensivity(30.f), m_is_enabled(true), m_is_shadows_enabled(false)
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

	const AABB_3Df& LightComponent::bounding_box() const
	{
		return m_bounds;
	}

	const Color3& LightComponent::light_color() const
	{
		return m_light_color;
	}

	float LightComponent::intensivity() const
	{
		return m_intensivity;
	}

	bool LightComponent::is_enabled() const
	{
		return m_is_enabled;
	}

	bool LightComponent::is_shadows_enabled() const
	{
		return m_is_shadows_enabled;
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
			m_shadow_map = ShadowMapPool::instance()->request_surface();
		}
		else if (!enabled && m_shadow_map != nullptr)
		{
			ShadowMapPool::instance()->return_surface(m_shadow_map.ptr());
		}

		return submit_light_info_render_thread();
	}

	class UpdateLightInfoCommand : public Task<UpdateLightInfoCommand>
	{
	private:
		AABB_3Df m_bounds;
		Color3 m_light_color;
		float m_intensivity;
		bool m_is_enabled;
		bool m_is_shadows_enabled;
		LightComponentProxy* m_proxy;

	public:
		UpdateLightInfoCommand(LightComponent* component)
		    : m_bounds(component->bounding_box()), m_light_color(component->light_color()),
		      m_intensivity(component->intensivity()), m_is_enabled(component->is_enabled()),
		      m_is_shadows_enabled(component->is_shadows_enabled()), m_proxy(component->proxy())
		{}

		void execute() override
		{
			m_proxy->bounding_box(m_bounds)
			        .light_color(m_light_color)
			        .intensivity(m_intensivity)
			        .is_enabled(m_is_enabled)
			        .is_shadows_enabled(m_is_shadows_enabled);
		}
	};

	LightComponent& LightComponent::submit_light_info_render_thread()
	{
		render_thread()->create_task<UpdateLightInfoCommand>(this);
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

	LightComponent::~LightComponent()
	{}
}// namespace Engine
