#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>
#include <Render/editor_scene_renderer.hpp>
#include <editor_resources.hpp>


namespace Engine
{
    extern void render_editor_grid(SceneRenderer* renderer, RenderTargetBase*, SceneLayer* layer);
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

    static void render_spot_light_overlay(SpotLightComponent* component)
    {
        auto proxy         = component->proxy();
        Material* material = EditorResources::spot_light_overlay_material;

        auto model_param   = reinterpret_cast<Mat4MaterialParameter*>(material->find_parameter(Name::model));
        model_param->param = proxy->world_transform().matrix();

        auto height_param   = reinterpret_cast<FloatMaterialParameter*>(material->find_parameter(Name::height));
        height_param->param = proxy->height();

        auto radius_param   = reinterpret_cast<FloatMaterialParameter*>(material->find_parameter(Name::radius));
        radius_param->param = proxy->radius();

        auto color_param   = reinterpret_cast<Vec4MaterialParameter*>(material->find_parameter(Name::color));
        color_param->param = Vector4D(1.f);

        material->apply();

        EditorResources::spot_light_overlay_positions->rhi_bind(0);
        engine_instance->rhi()->draw(EditorResources::spot_light_overlay_positions->buffer.size());
    }

    class OverlaySceneLayer : public SceneLayer
    {
    public:
        Set<LightComponent*> m_light_components;

        SceneLayer::Type type() const override
        {
            return Type::Custom;
        }

        OverlaySceneLayer& clear() override
        {
            SceneLayer::clear();
            m_light_components.clear();
            return *this;
        }

        OverlaySceneLayer& render(SceneRenderer* renderer, RenderTargetBase* rt) override
        {
            renderer->begin_rendering_target(SceneColorOutput::current_target());

            lines.render(renderer->scene_view());

            for (LightComponent* component : m_light_components)
            {
                render_light_sprite(EditorResources::light_sprite, component, renderer->scene_view());

                if (component->actor()->is_selected())
                {
                    if (SpotLightComponent* spot_light = component->instance_cast<SpotLightComponent>())
                    {
                        render_spot_light_overlay(spot_light);
                    }
                }
            }

            render_editor_grid(renderer, rt, this);

            renderer->end_rendering_target();
            return *this;
        }
    };

    EditorSceneRenderer::EditorSceneRenderer()
    {
        m_overlay_layer = post_process_layer()->create_next<OverlaySceneLayer>("Overlay Layer");
    }

    EditorSceneRenderer& EditorSceneRenderer::add_component(LightComponent* component, Scene* scene)
    {
        SceneRenderer::add_component(component, scene);
        m_overlay_layer->m_light_components.insert(component);
        return *this;
    }

    EditorSceneRenderer& EditorSceneRenderer::render_component(PrimitiveComponent* component, RenderTargetBase* rt,
                                                               SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);

        Actor* owner = component->actor();
        if (owner == nullptr)
            return *this;

        if (owner->is_selected() && owner->scene_component() == component)
        {
            component->proxy()->bounding_box().write_to_batcher(m_overlay_layer->lines, {255, 0, 0, 255});
        }

        return *this;
    }

    EditorSceneRenderer& EditorSceneRenderer::render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        SceneRenderer::render_component(component, rt, layer);

        Actor* owner = component->actor();
        if (owner == nullptr)
            return *this;

        if (owner->is_selected() && owner->scene_component() == component && !component->is_instance_of<SpotLightComponent>())
        {
            component->proxy()->bounding_box().write_to_batcher(layer->lines, {255, 0, 0, 255});
        }

        return *this;
    }
}// namespace Engine
