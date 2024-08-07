#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Render/command_buffer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_engine_class_default_init(DirectionalLightComponent, 0);

    static Vector3D get_direction(const Transform& world_transform)
    {
        return -world_transform.up_vector();
    }

    Vector3D DirectionalLightComponentProxy::direction() const
    {
        return get_direction(world_transform());
    }

    Vector3D DirectionalLightComponent::direction() const
    {
        return get_direction(world_transform());
    }

    DirectionalLightComponent::Type DirectionalLightComponent::light_type() const
    {
        return Type::Directional;
    }

    ActorComponentProxy* DirectionalLightComponent::create_proxy()
    {
        return new DirectionalLightComponentProxy();
    }

    DirectionalLightComponentProxy* DirectionalLightComponent::proxy() const
    {
        return typed_proxy<DirectionalLightComponentProxy>();
    }


    implement_empty_rendering_methods_for(DirectionalLightComponent);

#define get_param(param_name, type) reinterpret_cast<type*>(material->find_parameter(Name::param_name));
    ColorSceneRenderer& ColorSceneRenderer::render_component(DirectionalLightComponent* component)
    {
        render_base_component(component);

        CommandBufferLayer* layer             = deferred_lighting_layer();
        DirectionalLightComponentProxy* proxy = component->proxy();

        if (!scene_view().show_flags().has_all(ShowFlags::DirectionalLights) || !proxy->is_enabled() ||
            !component->leaf_class_is<DirectionalLightComponent>())
            return *this;

        Material* material = DefaultResources::Materials::directional_light;

        Vec3MaterialParameter* color_parameter        = get_param(color, Vec3MaterialParameter);
        Vec3MaterialParameter* direction_parameter    = get_param(direction, Vec3MaterialParameter);
        FloatMaterialParameter* intensivity_parameter = get_param(intensivity, FloatMaterialParameter);

        if (color_parameter)
        {
            layer->update_variable(color_parameter->param, proxy->light_color());
        }

        if (direction_parameter)
        {
            layer->update_variable(direction_parameter->param, proxy->direction());
        }

        if (intensivity_parameter)
        {
            layer->update_variable(intensivity_parameter->param, proxy->intensivity());
        }

        layer->bind_material(material, nullptr);
        layer->bind_vertex_buffer(DefaultResources::Buffers::screen_position, 0, 0);
        layer->draw(6, 0);
        return *this;
    }

    DirectionalLightComponent& DirectionalLightComponent::render(class SceneRenderer* renderer)
    {
        renderer->render_component(this);
        return *this;
    }
}// namespace Engine
