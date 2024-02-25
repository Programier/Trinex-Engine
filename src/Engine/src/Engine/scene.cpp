#include <Core/render_thread.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>


namespace Engine
{

    const Name SceneLayer::name_clear_render_targets = "Clear Render Targets";
    const Name SceneLayer::name_base_pass            = "Base Pass";
    const Name SceneLayer::name_light_pass           = "Light Pass";
    const Name SceneLayer::name_scene_output_pass    = "Scene Output Pass";
    const Name SceneLayer::name_post_process         = "Post Process";

    SceneLayer::SceneLayer(const Name& name) : m_name(name)
    {}

    SceneLayer::~SceneLayer()
    {
        if (m_next)
            delete m_next;
        m_next = nullptr;

        if (m_parent)
        {
            m_parent->m_next = nullptr;
            m_parent         = nullptr;
        }
    }

    SceneLayer& SceneLayer::clear()
    {
        for (PrimitiveComponent* component : m_components)
        {
            component->m_layer = nullptr;
        }
        m_components.clear();
        return *this;
    }

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        for (auto& method : methods_callback)
        {
            (renderer->*method)(viewport, this);
        }

        for (PrimitiveComponent* component : m_components)
        {
            component->render(renderer, viewport, this);
        }

        for (auto& func : function_callbacks)
        {
            func(renderer, viewport, this);
        }
        return *this;
    }

    SceneLayer* SceneLayer::parent() const
    {
        return m_parent;
    }

    SceneLayer* SceneLayer::next() const
    {
        return m_next;
    }

    const Name& SceneLayer::name() const
    {
        return m_name;
    }

    void SceneLayer::destroy()
    {
        if (m_parent)
        {
            m_parent->m_next = m_next;
        }

        if (m_next)
        {
            m_next->m_parent = m_parent;
        }

        delete this;
    }

    SceneLayer* SceneLayer::find(const Name& name)
    {
        SceneLayer* current = this;

        while (current)
        {
            if (current->m_name == name)
                return current;
            current = current->m_next;
        }

        current = this->m_parent;

        while (current)
        {
            if (current->m_name == name)
                return current;
            current = current->m_parent;
        }

        return nullptr;
    }

    SceneLayer* SceneLayer::create_next(const Name& name)
    {
        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = new SceneLayer(name);
        new_layer->m_parent = this;
        new_layer->m_next   = m_next;
        m_next              = new_layer;
        return new_layer;
    }

    SceneLayer* SceneLayer::create_parent(const Name& name)
    {
        if (!m_can_create_parent)
            return nullptr;

        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer           = new SceneLayer(name);
        new_layer->m_parent = m_parent;
        new_layer->m_next   = this;
        m_parent            = new_layer;
        return new_layer;
    }


    static void add_component_render_thread(PrimitiveComponent* component, Vector<PrimitiveComponent*>* components,
                                            Vector<Index>* free_indices)
    {
        if (!free_indices->empty())
        {
            components->at(free_indices->back()) = component;
            free_indices->pop_back();
        }
        else
        {
            components->push_back(component);
        }
    }

    static void remove_component_render_thread(PrimitiveComponent* component, Vector<PrimitiveComponent*>* components,
                                               Vector<Index>* free_indices)
    {
        for (size_t i = 0, j = components->size(); i < j; ++i)
        {
            if (components->at(i) == component)
            {
                free_indices->push_back(i);
                components->at(i) = nullptr;
            }
        }
    }

    struct SceneLayerAddComponent : public ExecutableObject {
        PrimitiveComponent* component;
        Vector<PrimitiveComponent*>* components;
        Vector<Index>* free_indices;

        SceneLayerAddComponent(PrimitiveComponent* component, Vector<PrimitiveComponent*>* components,
                               Vector<Index>* free_indices)
            : component(component), components(components), free_indices(free_indices)
        {}

        int_t execute()
        {
            add_component_render_thread(component, components, free_indices);
            return sizeof(SceneLayerAddComponent);
        }
    };

    struct SceneLayerRemoveComponent : public ExecutableObject {
        PrimitiveComponent* component;
        Vector<PrimitiveComponent*>* components;
        Vector<Index>* free_indices;

        SceneLayerRemoveComponent(PrimitiveComponent* component, Vector<PrimitiveComponent*>* components,
                                  Vector<Index>* free_indices)
            : component(component), components(components), free_indices(free_indices)
        {}

        int_t execute()
        {
            remove_component_render_thread(component, components, free_indices);
            return sizeof(SceneLayerAddComponent);
        }
    };


    SceneLayer& SceneLayer::add_component(PrimitiveComponent* component)
    {
        if (!component || component->m_layer == this)
            return *this;

        if (component->m_layer != nullptr)
        {
            component->m_layer->remove_component(component);
        }

        m_components.insert(component);
        return *this;
    }

    SceneLayer& SceneLayer::remove_component(PrimitiveComponent* component)
    {
        if (!component || component->m_layer != this)
            return *this;

        m_components.erase(component);
        component->m_layer = nullptr;
        return *this;
    }
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
        m_clear_layer->methods_callback.push_back(&SceneRenderer::clear_render_targets);

        m_base_pass_layer = m_clear_layer->create_next(SceneLayer::name_base_pass);
        m_base_pass_layer->methods_callback.push_back(&SceneRenderer::begin_rendering_base_pass);

        m_lighting_layer = m_base_pass_layer->create_next(SceneLayer::name_light_pass);
        m_lighting_layer->methods_callback.push_back(&SceneRenderer::begin_lighting_pass);

        m_scene_output = m_lighting_layer->create_next(SceneLayer::name_scene_output_pass);
        m_scene_output->methods_callback.push_back(&SceneRenderer::begin_scene_output_pass);

        m_post_process_layer = m_scene_output->create_next(SceneLayer::name_post_process);
        m_post_process_layer->methods_callback.push_back(&SceneRenderer::begin_postprocess_pass);
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
