#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{

    static FORCE_INLINE Vector3D calc_spot_light_direction(const Transform& transform)
    {
        // Using down vector for direction
        return transform.up_vector() * -1.f;
    }

    implement_engine_class(SpotLightComponent, 0);
    implement_initialize_class(SpotLightComponent)
    {
        Class* self                 = static_class_instance();
        static auto on_data_changed = [](void* object) {
            SpotLightComponent* component = reinterpret_cast<SpotLightComponent*>(object);
            component->on_angle_changed();
        };

        auto angle_property = new FloatProperty("Angle", "Angle of this spot light", &This::m_angle);
        angle_property->on_prop_changed.push(on_data_changed);

        self->add_properties(angle_property);
    }

    float SpotLightComponentProxy::angle() const
    {
        return m_angle;
    }

    float SpotLightComponentProxy::cos_cutoff() const
    {
        return m_cos_cutoff;
    }

    SpotLightComponentProxy& SpotLightComponentProxy::angle(float value)
    {
        m_angle      = glm::radians(value);
        m_cos_cutoff = glm::cos(m_angle / 2.f);
        return *this;
    }

    Vector3D SpotLightComponentProxy::direction() const
    {
        return calc_spot_light_direction(world_transform());
    }

    SpotLightComponent::SpotLightComponent() : m_angle(60.f)
    {}

    class UpdateSpotLightAngleCommand : public ExecutableObject
    {
    private:
        float m_angle;
        SpotLightComponentProxy* m_proxy;

    public:
        UpdateSpotLightAngleCommand(SpotLightComponent* component) : m_angle(component->angle()), m_proxy(component->proxy())
        {}

        int_t execute() override
        {
            m_proxy->angle(m_angle);
            return sizeof(UpdateSpotLightAngleCommand);
        }
    };

    void SpotLightComponent::on_angle_changed()
    {
        m_angle = glm::clamp(m_angle, 0.f, 180.f);
        render_thread()->insert_new_task<UpdateSpotLightAngleCommand>(this);
    }

    float SpotLightComponent::angle() const
    {
        return m_angle;
    }

    SpotLightComponent& SpotLightComponent::angle(float value)
    {
        m_angle = value;
        return *this;
    }

    Vector3D SpotLightComponent::direction() const
    {
        return calc_spot_light_direction(world_transform());
    }

    SpotLightComponentProxy* SpotLightComponent::proxy() const
    {
        return typed_proxy<SpotLightComponentProxy>();
    }

    LightComponent::Type SpotLightComponent::light_type() const
    {
        return LightComponent::Type::Spot;
    }

    SceneRenderer& SceneRenderer::add_component(SpotLightComponent* component, Scene* scene)
    {
        add_base_component(component, scene);

        if (component->leaf_class_is<SpotLightComponent>())
        {
            deferred_lighting_layer()->add_light(component);
        }
        return *this;
    }

    SpotLightComponent& SpotLightComponent::spawned()
    {
        Super::spawned();
        on_angle_changed();
        return *this;
    }

    SpotLightComponent& SpotLightComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

#define get_param(param_name, type) reinterpret_cast<type*>(material->find_parameter(Name::param_name));
    SceneRenderer& SceneRenderer::render_component(SpotLightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        render_base_component(component, rt, layer);

        if (!component->is_enabled)
            return *this;

        Material* material = DefaultResources::spot_light_material;

        Vec3MaterialParameter* color_parameter         = get_param(color, Vec3MaterialParameter);
        Vec3MaterialParameter* ambient_color_parameter = get_param(ambient_color, Vec3MaterialParameter);
        FloatMaterialParameter* intensivity_parameter  = get_param(intensivity, FloatMaterialParameter);
        FloatMaterialParameter* cutoff_parameter       = get_param(cutoff, FloatMaterialParameter);
        Vec3MaterialParameter* location_parameter      = get_param(location, Vec3MaterialParameter);
        Vec3MaterialParameter* direction_parameter     = get_param(direction, Vec3MaterialParameter);

        if (color_parameter)
        {
            color_parameter->param = component->light_color;
        }

        if (ambient_color_parameter)
        {
            ambient_color_parameter->param = scene()->environment.ambient_color;
        }

        if (intensivity_parameter)
        {
            intensivity_parameter->param = component->intensivity;
        }

        auto proxy = component->proxy();

        if (location_parameter)
        {
            location_parameter->param = proxy->world_transform().location();
        }

        if (direction_parameter)
        {
            direction_parameter->param = proxy->direction();
        }

        if (cutoff_parameter)
        {
            cutoff_parameter->param = proxy->cos_cutoff();
        }

        material->apply();
        DefaultResources::screen_position_buffer->rhi_bind(0, 0);
        engine_instance->rhi()->draw(6);
        return *this;
    }

    SpotLightComponent& SpotLightComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt,
                                                   class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }

    ActorComponentProxy* SpotLightComponent::create_proxy()
    {
        return new SpotLightComponentProxy();
    }
}// namespace Engine
