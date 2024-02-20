#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    implement_class(StaticMeshComponent, Engine, 0);
    implement_default_initialize_class(StaticMeshComponent);

    StaticMeshComponent& StaticMeshComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        if (mesh && mesh->material)
        {
            if (Pipeline* pipeline = mesh->material->material()->pipeline)
            {
                if (pipeline->render_pass == RenderPassType::SceneOutput)
                {
                    scene->post_process_layer()->add_component(this);
                }
            }
        }
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer*, class RenderViewport*, class SceneLayer*)
    {
        mesh->material->apply(this);
        mesh->positions[0]->rhi_bind(0, 0);
        mesh->indices->rhi_bind(0);

        engine_instance->rhi()->draw_indexed(mesh->indices->elements_count(), 0);
        return *this;
    }
}// namespace Engine
