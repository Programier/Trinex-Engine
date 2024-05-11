#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/static_mesh_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/Render/command_buffer.hpp>
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

    ColorSceneRenderer& ColorSceneRenderer::render_component(StaticMeshComponent* component)
    {
        render_base_component(component);

        if (!scene_view().show_flags().has_all(ShowFlags::StaticMesh))
            return *this;

        StaticMesh* mesh   = component->mesh;
        auto& camera_view  = scene_view().camera_view();
        float inv_distance = 1.f / glm::min(glm::distance(component->proxy()->world_transform().location(), camera_view.location),
                                            camera_view.far_clip_plane);
        auto& lods         = mesh->lods;
        Index lod_index    = glm::min(static_cast<Index>(static_cast<float>(lods.size()) * inv_distance), lods.size() - 1);
        auto& lod          = lods[lod_index];

        for (auto& material : mesh->materials)
        {
            if (material.policy != policy_id() || material.material == nullptr ||
                static_cast<Index>(material.surface_index) > lod.surfaces.size())
            {
                continue;
            }

            auto layer = base_pass_layer();

            material.material->apply(component);
            layer->bind_material(material.material, component);

            VertexShader* shader = material.material->material()->pipeline->vertex_shader();

            for (Index i = 0, count = shader->input_attributes.size(); i < count; ++i)
            {
                auto& attribute      = shader->input_attributes[i];
                VertexBuffer* buffer = lod.find_vertex_buffer(attribute.semantic, attribute.semantic_index);

                if (buffer)
                {
                    layer->bind_vertex_buffer(buffer, i, 0);
                }
            }

            auto& surface = lod.surfaces[material.surface_index];

            if (lod.indices->elements_count() > 0)
            {
                lod.indices->rhi_bind();
                layer->draw_indexed(surface.vertices_count, surface.first_index, surface.base_vertex_index);
            }
            else
            {
                layer->draw(surface.vertices_count, surface.base_vertex_index);
            }
        }

        return *this;
    }

    StaticMeshComponent& StaticMeshComponent::render(class SceneRenderer* renderer)
    {
        renderer->render_component(this);
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
