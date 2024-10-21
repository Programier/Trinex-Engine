#include <Core/base_engine.hpp>
#include <Core/property.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	static const AABB_3Df light_bounds({-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f});

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

	implement_engine_class(LightComponent, 0)
	{
		Refl::Class* self = static_class_instance();

		static auto on_props_changed = [](void* object) {
			LightComponent* component = reinterpret_cast<LightComponent*>(object);
			component->submit_light_info_render_thread();
		};

		auto is_enabled_prop = new ClassProperty("Is Enabled", "Is light enabled", &This::m_is_enabled);
		auto shadows_prop =
		        new ClassProperty("Enable Shadows", "The light source can cast real-time shadows", &This::m_is_shadows_enabled);
		auto color_prop = new ClassProperty("Color", "Color of this light", &This::m_light_color, Name::none, Property::IsColor);
		auto intensivity_prop = new ClassProperty("Intensivity", "Intensivity of this light", &This::m_intensivity);

		is_enabled_prop->on_prop_changed.push(on_props_changed);
		shadows_prop->on_prop_changed.push(on_props_changed);
		color_prop->on_prop_changed.push(on_props_changed);
		intensivity_prop->on_prop_changed.push(on_props_changed);

		self->add_properties(is_enabled_prop, shadows_prop, color_prop, intensivity_prop);
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
		return submit_light_info_render_thread();
	}

	class UpdateLightInfoCommand : public ExecutableObject
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

		int_t execute() override
		{
			m_proxy->bounding_box(m_bounds)
			        .light_color(m_light_color)
			        .intensivity(m_intensivity)
			        .is_enabled(m_is_enabled)
			        .is_shadows_enabled(m_is_shadows_enabled);
			return sizeof(UpdateLightInfoCommand);
		}
	};

	LightComponent& LightComponent::submit_light_info_render_thread()
	{
		render_thread()->insert_new_task<UpdateLightInfoCommand>(this);
		return *this;
	}

	LightComponent& LightComponent::update_bounding_box()
	{
		m_bounds = AABB_3Df(light_bounds).center(world_transform().location());
		return submit_light_info_render_thread();
	}

	LightComponent::~LightComponent()
	{}
}// namespace Engine
