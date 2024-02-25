#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/scene.hpp>


namespace Engine
{
    class AddPrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* m_octree;
        PrimitiveComponent* m_primitive;
        AABB_3Df m_box;

    public:
        AddPrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : m_octree(octree), m_primitive(primitive), m_box(box)
        {}

        int_t execute() override
        {
            m_octree->push(m_box, m_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    class RemovePrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* m_octree;
        PrimitiveComponent* m_primitive;
        AABB_3Df m_box;

    public:
        RemovePrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : m_octree(octree), m_primitive(primitive), m_box(box)
        {}

        int_t execute() override
        {
            m_octree->remove(m_box, m_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    Scene::Scene()
    {
        m_root_component = Object::new_instance_named<SceneComponent>("Root");

        m_root_layer                      = new SceneLayer("Root Layer");
        m_root_layer->m_can_create_parent = false;

        m_clear_layer = m_root_layer->create_next(SceneLayer::name_clear_render_targets);
        m_clear_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::clear_render_targets);

        m_base_pass_layer = m_clear_layer->create_next(SceneLayer::name_base_pass);
        m_base_pass_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_rendering_base_pass);
        m_base_pass_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_lighting_layer = m_base_pass_layer->create_next(SceneLayer::name_light_pass);
        m_lighting_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_lighting_pass);
        m_lighting_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_scene_output = m_lighting_layer->create_next(SceneLayer::name_scene_output_pass);
        m_scene_output->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_scene_output_pass);
        m_scene_output->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_post_process_layer = m_scene_output->create_next(SceneLayer::name_post_process);
        m_post_process_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_postprocess_pass);
        m_post_process_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);
    }


    Scene& Scene::build_views_internal(SceneRenderer* renderer, SceneOctree::Node* node)
    {
        for (PrimitiveComponent* component : node->values)
        {
            component->add_to_scene_layer(this, renderer);
        }

        for (byte i = 0; i < 8; i++)
        {
            auto child = node->child_at(i);

            if (child)
            {
                build_views_internal(renderer, child);
            }
        }
        return *this;
    }

    Scene& Scene::build_views(SceneRenderer* renderer)
    {
        for (auto layer = root_layer(); layer; layer = layer->next())
        {
            layer->clear();
        }

        return build_views_internal(renderer, m_octree_render_thread.root_node());
    }

    Scene& Scene::add_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<AddPrimitiveTask>(&m_octree_render_thread, primitive, primitive->bounding_box());
        m_octree.push(primitive->bounding_box(), primitive);
        return *this;
    }

    Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<RemovePrimitiveTask>(&m_octree_render_thread, primitive, primitive->bounding_box());
        m_octree.remove(primitive->bounding_box(), primitive);
        return *this;
    }

    Scene& Scene::add_light(LightComponent* light)
    {
        return *this;
    }

    Scene& Scene::remove_light(LightComponent* light)
    {
        return *this;
    }

    SceneComponent* Scene::root_component() const
    {
        return m_root_component.ptr();
    }

    const Scene::SceneOctree& Scene::octree() const
    {
        if (is_in_render_thread())
        {
            return m_octree_render_thread;
        }
        else
        {
            return m_octree;
        }
    }

    Scene::~Scene()
    {
        delete m_root_layer;
        m_root_layer = nullptr;
    }
}// namespace Engine
