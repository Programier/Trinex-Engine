#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
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

    StaticMeshComponent& StaticMeshComponent::add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer)
    {
        if (mesh && mesh->material && !mesh->lods.empty())
        {
            if (Pipeline* pipeline = mesh->material->material()->pipeline)
            {
                RenderPassType type = pipeline->render_pass_type();
                if (type == RenderPassType::OneAttachentOutput)
                {
                    scene->scene_output_layer()->add_component(this);
                }
                else if (type == RenderPassType::GBuffer)
                {
                    scene->base_pass_layer()->add_component(this);
                }
            }
        }
        else if (engine_instance->is_editor())
        {
            scene->scene_output_layer()->add_component(this);
        }
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer* renderer, class RenderTargetBase* rt,
                                                     class SceneLayer* layer)
    {
        if (mesh && mesh->material)
        {
            auto& camera_view  = renderer->scene_view().camera_view();
            float inv_distance = 1.f / glm::min(glm::distance(world_transform().location(), camera_view.location),
                                                camera_view.far_clip_plane);
            auto& lods         = mesh->lods;
            Index lod_index    = glm::min(static_cast<Index>(static_cast<float>(lods.size()) * inv_distance), lods.size() - 1);

            auto& lod = lods[lod_index];

            mesh->material->apply(this);
            VertexShader* shader = mesh->material->material()->pipeline->vertex_shader();

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
        }
        else
        {
            Super::render(renderer, rt, layer);
        }
        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::update_bounding_box()
    {
        if (mesh)
        {
            m_bounding_box = mesh->bounds.apply_transform(world_transform().matrix());
        }
        else
        {
            Super::update_bounding_box();
        }
        return *this;
    }
}// namespace Engine
