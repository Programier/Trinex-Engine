#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/shader.hpp>

namespace Engine
{
    implement_class(StaticMeshComponent, Engine, 0);
    implement_default_initialize_class(StaticMeshComponent);

    StaticMeshComponent& StaticMeshComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        if (mesh && mesh->material && !mesh->lods.empty())
        {
            if (Pipeline* pipeline = mesh->material->material()->pipeline)
            {
                if (pipeline->render_pass == RenderPassType::SceneOutput)
                {
                    scene->post_process_layer()->add_component(this);
                }
                else if (pipeline->render_pass == RenderPassType::GBuffer)
                {
                    scene->base_pass_layer()->add_component(this);
                }
            }
        }
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer* renderer, class RenderViewport*, class SceneLayer*)
    {
        auto& camera_view  = renderer->camera_view();
        float inv_distance = 1.f / glm::min(glm::distance(transform_render_thread.global_location(), camera_view.location),
                                            camera_view.far_clip_plane);
        auto& lods         = mesh->lods;
        Index lod_index    = glm::min(static_cast<Index>(static_cast<float>(lods.size()) * inv_distance), lods.size() - 1);

        auto& lod = lods[lod_index];

        mesh->material->apply(this);
        VertexShader* shader = mesh->material->material()->pipeline->vertex_shader;

        size_t vertices = Constants::max_size;

        for (Index i = 0, count = shader->attributes.size(); i < count; ++i)
        {
            auto& attribute      = shader->attributes[i];
            VertexBuffer* buffer = lod.find_vertex_buffer(attribute.semantic, attribute.semantic_index);

            if (buffer)
            {
                buffer->rhi_bind(i, 0);

                if (attribute.rate == VertexAttributeInputRate::Vertex)
                {
                    vertices = glm::min(buffer->elements_count(), vertices);
                }
            }
        }

        RHI* rhi = engine_instance->rhi();

        if (lod.indices->elements_count() > 0)
        {
            lod.indices->rhi_bind(0);
            rhi->draw_indexed(lod.indices->elements_count(), 0);
        }
        else if (vertices != Constants::max_size)
        {
            rhi->draw(vertices);
        }
        return *this;
    }
}// namespace Engine