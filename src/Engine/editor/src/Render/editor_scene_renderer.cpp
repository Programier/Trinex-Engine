#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Render/editor_scene_renderer.hpp>
#include <editor_resources.hpp>

namespace Engine
{
    EditorSceneRenderer& EditorSceneRenderer::render_component(PrimitiveComponent* component, RenderTargetBase* rt,
                                                               SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);

        Actor* owner = component->actor();
        if (owner == nullptr)
            return *this;

        if (owner->is_selected() && owner->scene_component() == component)
        {
            component->proxy()->bounding_box().write_to_batcher(layer->lines, {255, 0, 0, 255});
        }
        return *this;
    }

    static void render_light_sprite(Texture2D* texture, LightComponent* component, const SceneView& view)
    {
        Material* material                 = DefaultResources::sprite_material;
        PositionVertexBuffer* vertex_bufer = DefaultResources::screen_position_buffer;

        if (Mat4MaterialParameter* parameter = reinterpret_cast<Mat4MaterialParameter*>(material->find_parameter(Name::model)))
        {
            Transform transform = component->proxy()->world_transform();
            transform.scale({0.5, 0.5, 0.5});
            transform.look_at(view.camera_view().location, Constants::OY);
            parameter->param = transform.matrix();
        }

        BindingMaterialParameter* texture_parameter =
                reinterpret_cast<BindingMaterialParameter*>(material->find_parameter(Name::texture));
        Texture* tmp = nullptr;

        if (texture_parameter && texture)
        {
            tmp = texture_parameter->texture_param();
            texture_parameter->texture_param(texture);
        }

        material->apply(component);
        vertex_bufer->rhi_bind(0, 0);
        engine_instance->rhi()->draw(6);

        if (texture_parameter && texture)
        {
            texture_parameter->texture_param(tmp);
        }
    }

    EditorSceneRenderer& EditorSceneRenderer::render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);

        Actor* owner = component->actor();
        if (owner == nullptr)
            return *this;

        if (owner->is_selected() && owner->scene_component() == component)
        {
            component->proxy()->bounding_box().write_to_batcher(layer->lines, {255, 0, 0, 255});
        }

        return *this;
    }


    EditorSceneRenderer& EditorSceneRenderer::render_component(PointLightComponent* component, RenderTargetBase* rt,
                                                               SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);
        render_light_sprite(EditorResources::light_sprite, component, scene_view());
        return *this;
    }

    EditorSceneRenderer& EditorSceneRenderer::render_component(SpotLightComponent* component, RenderTargetBase* rt,
                                                               SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);
        render_light_sprite(EditorResources::light_sprite, reinterpret_cast<LightComponent*>(component), scene_view());
        return *this;
    }
}// namespace Engine
