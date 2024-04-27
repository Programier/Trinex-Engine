#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/sprite_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/texture_2D.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Engine
{
    implement_engine_class(SpriteComponent, 0);
    implement_initialize_class(SpriteComponent)
    {
        Class* self    = This::static_class_instance();
        Property* prop = new ObjectReferenceProperty("Texture", "Sprite texture", &This::m_texture);
        prop->on_prop_changed.push([](void* object) { reinterpret_cast<SpriteComponent*>(object)->on_transform_changed(); });
        self->add_property(prop);
    }

    SceneRenderer& SceneRenderer::add_component(SpriteComponent* component, Scene* scene)
    {
        scene_output_layer()->add_component(component);
        return *this;
    }

    SpriteComponent& SpriteComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

    Texture2D* SpriteComponent::texture() const
    {
        return m_texture;
    }

    SpriteComponent& SpriteComponent::texture(Texture2D* texture)
    {
        m_texture = texture;
        on_transform_changed();
        return *this;
    }

    SpriteComponent& SpriteComponent::update_bounding_box()
    {
        if (m_texture)
        {
            static AABB_3Df default_aabb({-.5, -.5, -.5}, {.5f, .5f, .5f});
            m_bounding_box = default_aabb.apply_transform(world_transform().matrix());
        }
        else
        {
            const Vector3D& location = world_transform().location();
            m_bounding_box.minmax(location, location);
        }

        submit_bounds_to_render_thread();
        return *this;
    }


    static FORCE_INLINE Matrix4f rotate_sprite(Transform input_transform, const SceneView& view)
    {
        return input_transform.look_at(view.camera_view().location, Constants::OY).matrix();
    }

    SceneRenderer& SceneRenderer::render_component(SpriteComponent* component, RenderTargetBase* rt, SceneLayer* layer)
    {
        render_component(static_cast<SpriteComponent::Super*>(component), rt, layer);
        Material* material                 = DefaultResources::sprite_material;
        PositionVertexBuffer* vertex_bufer = DefaultResources::screen_position_buffer;
        if (Mat4MaterialParameter* parameter = reinterpret_cast<Mat4MaterialParameter*>(material->find_parameter(Name::model)))
        {
            Matrix4f model   = rotate_sprite(component->proxy()->world_transform(), scene_view());
            parameter->param = model;
        }

        BindingMaterialParameter* texture_parameter =
                reinterpret_cast<BindingMaterialParameter*>(material->find_parameter(Name::texture));
        Texture* tmp     = nullptr;
        Texture* current = reinterpret_cast<Texture*>(component->texture());

        if (texture_parameter && current)
        {
            tmp = texture_parameter->texture_param();
            texture_parameter->texture_param(current);
        }

        material->apply(component);
        vertex_bufer->rhi_bind(0, 0);
        engine_instance->rhi()->draw(6);

        if (texture_parameter && current)
        {
            texture_parameter->texture_param(tmp);
        }
        return *this;
    }

    SpriteComponent& SpriteComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt, class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }


}// namespace Engine
