#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{
    implement_class(StaticMeshComponent, Engine, 0);
    implement_initialize_class(StaticMeshComponent)
    {
        Class* self = This::static_class_instance();
        self->add_property(new ObjectReferenceProperty("Mesh", "Mesh of this component", &This::mesh));
    }

    StaticMeshComponent& StaticMeshComponent::update(float dt)
    {
        Super::update(dt);
        return *this;
    }

    implement_empty_rendering_methods_for(StaticMeshComponent);

    ColorSceneRenderer& ColorSceneRenderer::add_component(StaticMeshComponent* component, Scene* scene)
    {
        add_base_component(component, scene);

        if (component->leaf_class_is<StaticMeshComponent>())
        {
            StaticMesh* mesh = component->mesh;
            if (mesh && !mesh->materials.empty() && !mesh->lods.empty())
            {
                base_pass_layer()->add_component(component);
            }
        }
        return *this;
    }


    static void render_static_mesh_component(StaticMeshComponent* component, PolicyID policy, const SceneView& scene_view)
    {
        StaticMesh* mesh   = component->mesh;
        auto& camera_view  = scene_view.camera_view();
        float inv_distance = 1.f / glm::min(glm::distance(component->proxy()->world_transform().location(), camera_view.location),
                                            camera_view.far_clip_plane);
        auto& lods         = mesh->lods;
        Index lod_index    = glm::min(static_cast<Index>(static_cast<float>(lods.size()) * inv_distance), lods.size() - 1);
        auto& lod          = lods[lod_index];

        for (auto& material : mesh->materials)
        {
            if (material.policy != policy || material.material == nullptr ||
                static_cast<Index>(material.surface_index) > lod.surfaces.size())
            {
                continue;
            }

            material.material->apply(component);
            VertexShader* shader = material.material->material()->pipeline->vertex_shader();

            for (Index i = 0, count = shader->attributes.size(); i < count; ++i)
            {
                auto& attribute      = shader->attributes[i];
                VertexBuffer* buffer = lod.find_vertex_buffer(attribute.semantic, attribute.semantic_index);

                if (buffer)
                {
                    buffer->rhi_bind(i, 0);
                }
            }

            auto& surface = lod.surfaces[material.surface_index];
            RHI* rhi      = engine_instance->rhi();

            if (lod.indices->elements_count() > 0)
            {
                lod.indices->rhi_bind();
                rhi->draw_indexed(surface.vertices_count, surface.first_index, surface.base_vertex_index);
            }
            else
            {
                rhi->draw(surface.vertices_count, surface.base_vertex_index);
            }
        }
    }

    ColorSceneRenderer& ColorSceneRenderer::render_component(StaticMeshComponent* component, class RenderTargetBase* rt,
                                                             class SceneLayer* layer)
    {
        render_base_component(component, rt, layer);
        render_static_mesh_component(component, policy_id(), scene_view());
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        renderer->add_component(this, scene);
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt,
                                                     class SceneLayer* layer)
    {
        renderer->render_component(this, rt, layer);
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::update_bounding_box()
    {
        if (mesh)
        {
            m_bounding_box = mesh->bounds.apply_transform(world_transform().matrix());
            submit_bounds_to_render_thread();
        }
        else
        {
            Super::update_bounding_box();
        }
        return *this;
    }
}// namespace Engine
