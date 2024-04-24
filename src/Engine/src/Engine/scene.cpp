#include <Core/render_thread.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>


namespace Engine
{
    template<typename OctreeType>
    class AddPrimitiveTask : public ExecutableObject
    {
        OctreeType* m_octree;
        OctreeType::ValueType m_primitive;
        AABB_3Df m_box;

    public:
        AddPrimitiveTask(OctreeType* octree, OctreeType::ValueType primitive, const AABB_3Df& box)
            : m_octree(octree), m_primitive(primitive), m_box(box)
        {}

        int_t execute() override
        {
            m_octree->push(m_box, m_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    template<typename OctreeType>
    class RemovePrimitiveTask : public ExecutableObject
    {
        OctreeType* m_octree;
        OctreeType::ValueType m_primitive;
        AABB_3Df m_box;

    public:
        RemovePrimitiveTask(OctreeType* octree, OctreeType::ValueType primitive, const AABB_3Df& box)
            : m_octree(octree), m_primitive(primitive), m_box(box)
        {}

        int_t execute() override
        {
            m_octree->remove(m_box, m_primitive);
            return sizeof(RemovePrimitiveTask);
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

        m_deferred_lighting_layer = m_base_pass_layer->create_next(SceneLayer::name_deferred_light_pass);
        m_deferred_lighting_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_deferred_lighting_pass);
        m_deferred_lighting_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_lighting_layer = m_deferred_lighting_layer->create_next(SceneLayer::name_light_pass);
        m_lighting_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_lighting_pass);
        m_lighting_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_scene_output = m_lighting_layer->create_next(SceneLayer::name_scene_output_pass);
        m_scene_output->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_scene_output_pass);
        m_scene_output->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);

        m_post_process_layer = m_scene_output->create_next(SceneLayer::name_post_process);
        m_post_process_layer->begin_render_methods_callbacks.push_back(&SceneRenderer::begin_postprocess_pass);
        m_post_process_layer->end_render_methods_callbacks.push_back(&SceneRenderer::end_rendering_target);
    }


    template<typename Node>
    static void build_views_internal(Scene* scene, SceneRenderer* renderer, Node* node)
    {
        for (auto component : node->values)
        {
            component->add_to_scene_layer(scene, renderer);
        }

        for (byte i = 0; i < 8; i++)
        {
            auto child = node->child_at(i);

            if (child)
            {
                build_views_internal(scene, renderer, child);
            }
        }
    }

    Scene& Scene::build_views(SceneRenderer* renderer)
    {
        for (auto layer = root_layer(); layer; layer = layer->next())
        {
            layer->clear();
        }

        build_views_internal(this, renderer, m_octree_render_thread.root_node());
        build_views_internal(this, renderer, m_light_octree_render_thread.root_node());
        return *this;
    }

    Scene& Scene::add_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<AddPrimitiveTask<Scene::PrimitiveOctree>>(&m_octree_render_thread, primitive,
                                                                                   primitive->bounding_box());
        m_octree.push(primitive->bounding_box(), primitive);
        return *this;
    }

    Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<RemovePrimitiveTask<Scene::PrimitiveOctree>>(&m_octree_render_thread, primitive,
                                                                                      primitive->bounding_box());
        m_octree.remove(primitive->bounding_box(), primitive);
        return *this;
    }

    struct UpdatePrimitiveTransformCommand : public ExecutableObject {
        AABB_3Df bounds;
        PrimitiveComponentProxy* proxy;

    public:
        UpdatePrimitiveTransformCommand(PrimitiveComponent* component)
            : bounds(component->bounding_box()), proxy(component->proxy())
        {}

        int_t execute() override
        {
            proxy->bounding_box(bounds);
            return sizeof(UpdatePrimitiveTransformCommand);
        }
    };

    Scene& Scene::update_primitive_transform(PrimitiveComponent* primitive)
    {
        remove_primitive(primitive);
        primitive->update_bounding_box();
        add_primitive(primitive);
        render_thread()->insert_new_task<UpdatePrimitiveTransformCommand>(primitive);
        return *this;
    }

    Scene& Scene::add_light(LightComponent* light)
    {
        render_thread()->insert_new_task<AddPrimitiveTask<Scene::LightOctree>>(&m_light_octree_render_thread, light,
                                                                               light->bounding_box());
        m_light_octree.push(light->bounding_box(), light);
        return *this;
    }

    Scene& Scene::remove_light(LightComponent* light)
    {
        render_thread()->insert_new_task<RemovePrimitiveTask<Scene::LightOctree>>(&m_light_octree_render_thread, light,
                                                                                  light->bounding_box());
        m_light_octree.remove(light->bounding_box(), light);
        return *this;
    }

    SceneComponent* Scene::root_component() const
    {
        return m_root_component.ptr();
    }

    const Scene::PrimitiveOctree& Scene::primitive_octree() const
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

    const Scene::LightOctree& Scene::light_octree() const
    {
        if (is_in_render_thread())
        {
            return m_light_octree_render_thread;
        }
        else
        {
            return m_light_octree;
        }
    }

    Scene::~Scene()
    {
        delete m_root_layer;
        m_root_layer = nullptr;
    }
}// namespace Engine
