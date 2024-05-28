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
        typename OctreeType::ValueType m_primitive;
        AABB_3Df m_box;

    public:
        AddPrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const AABB_3Df& box)
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
        typename OctreeType::ValueType m_primitive;
        AABB_3Df m_box;

    public:
        RemovePrimitiveTask(OctreeType* octree, typename OctreeType::ValueType primitive, const AABB_3Df& box)
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
    }


    template<typename Node>
    static void build_views_internal(SceneRenderer* renderer, Node* node)
    {
        for (auto component : node->values)
        {
            component->render(renderer);
        }

        for (byte i = 0; i < 8; i++)
        {
            auto child = node->child_at(i);

            if (child)
            {
                build_views_internal(renderer, child);
            }
        }
    }

    Scene& Scene::build_views(SceneRenderer* renderer)
    {
        build_views_internal(renderer, m_octree_render_thread.root_node());
        build_views_internal(renderer, m_light_octree_render_thread.root_node());
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

    Scene& Scene::update_primitive_transform(PrimitiveComponent* primitive)
    {
        remove_primitive(primitive);
        primitive->update_bounding_box();
        add_primitive(primitive);
        return *this;
    }

    Scene& Scene::update_light_transform(LightComponent* light)
    {
        remove_light(light);
        light->update_bounding_box();
        add_light(light);
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
    {}
}// namespace Engine
