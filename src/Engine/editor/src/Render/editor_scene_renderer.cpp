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
#include <Graphics/scene_render_targets.hpp>
#include <Render/editor_scene_renderer.hpp>
#include <editor_resources.hpp>

namespace Engine
{
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

            for(LightComponent* component : m_light_components)
            {
                render_light_sprite(EditorResources::light_sprite, component, renderer->scene_view());
            }

            renderer->end_rendering_target();
            return *this;
        }
    };

    EditorSceneRenderer::EditorSceneRenderer()
    {
        m_overlay_layer = post_process_layer()->create_next<OverlaySceneLayer>("Overlay Layer");
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

        if (owner->is_selected() && owner->scene_component() == component)
        {
            component->proxy()->bounding_box().write_to_batcher(layer->lines, {255, 0, 0, 255});
        }

        m_overlay_layer->m_light_components.insert(component);

        return *this;
    }
}// namespace Engine
