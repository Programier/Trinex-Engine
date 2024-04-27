#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
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
            component->on_data_changed();
        };

        auto radius_property = new FloatProperty("Radius", "Radius of this light", &This::m_radius);
        auto height_property = new FloatProperty("Height", "Height of this light", &This::m_height);

        radius_property->on_prop_changed.push(on_data_changed);
        height_property->on_prop_changed.push(on_data_changed);

        self->add_properties(radius_property);
        self->add_properties(height_property);
    }


    float SpotLightComponentProxy::radius() const
    {
        return m_radius;
    }

    float SpotLightComponentProxy::height() const
    {
        return m_height;
    }

    float SpotLightComponentProxy::cutoff() const
    {
        return m_cutoff;
    }

    SpotLightComponentProxy& SpotLightComponentProxy::radius(float value)
    {
        m_radius = value;
        return *this;
    }

    SpotLightComponentProxy& SpotLightComponentProxy::height(float value)
    {
        m_height = value;
        return *this;
    }

    SpotLightComponentProxy& SpotLightComponentProxy::cutoff(float value)
    {
        m_cutoff = value;
        return *this;
    }

    Vector3D SpotLightComponentProxy::direction() const
    {
        return calc_spot_light_direction(world_transform());
    }

    SpotLightComponent::SpotLightComponent() : m_radius(10.f), m_height(10.f), m_cutoff(0.f)
    {}

    class UpdateSpotLightDataCommand : public ExecutableObject
    {
    private:
        float m_radius;
        float m_height;
        float m_cutoff;
        SpotLightComponentProxy* m_proxy;

    public:
        UpdateSpotLightDataCommand(SpotLightComponent* component)
            : m_radius(component->radius()), m_height(component->height()), m_cutoff(component->cutoff()),
              m_proxy(component->proxy())
        {}

        int_t execute() override
        {
            m_proxy->radius(m_radius);
            m_proxy->height(m_height);
            m_proxy->cutoff(m_cutoff);
            return sizeof(UpdateSpotLightDataCommand);
        }
    };

    void SpotLightComponent::on_data_changed()
    {
        m_cutoff = m_height / glm::sqrt((m_height * m_height) + (m_radius * m_radius));
        render_thread()->insert_new_task<UpdateSpotLightDataCommand>(this);
    }

    float SpotLightComponent::radius() const
    {
        return m_radius;
    }

    float SpotLightComponent::height() const
    {
        return m_height;
    }

    float SpotLightComponent::cutoff() const
    {
        return m_cutoff;
    }


    SpotLightComponent& SpotLightComponent::radius(float value)
    {
        m_radius = value;
        on_data_changed();
        return *this;
    }

    SpotLightComponent& SpotLightComponent::height(float value)
    {
        m_height = value;
        on_data_changed();
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
        deferred_lighting_layer()->add_light(component);
        return *this;
    }

    SpotLightComponent& SpotLightComponent::spawned()
    {
        Super::spawned();
        on_data_changed();
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
        render_component(static_cast<SpotLightComponent::Super*>(component), rt, layer);

        if (!component->is_enabled)
            return *this;

        Material* material = DefaultResources::spot_light_material;

        Vec3MaterialParameter* color_parameter         = get_param(color, Vec3MaterialParameter);
        Vec3MaterialParameter* ambient_color_parameter = get_param(ambient_color, Vec3MaterialParameter);
        FloatMaterialParameter* intensivity_parameter  = get_param(intensivity, FloatMaterialParameter);
        Vec3MaterialParameter* location_parameter      = get_param(location, Vec3MaterialParameter);
        Vec3MaterialParameter* direction_parameter     = get_param(direction, Vec3MaterialParameter);
        FloatMaterialParameter* radius_parameter       = get_param(radius, FloatMaterialParameter);
        FloatMaterialParameter* height_parameter       = get_param(height, FloatMaterialParameter);
        FloatMaterialParameter* cutoff_parameter       = get_param(cutoff, FloatMaterialParameter);


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

        if (radius_parameter)
        {
            radius_parameter->param = proxy->radius();
        }

        if (height_parameter)
        {
            height_parameter->param = proxy->height();
        }

        if (cutoff_parameter)
        {
            cutoff_parameter->param = proxy->cutoff();
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
