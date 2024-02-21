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

    SceneLayer::SceneLayer(const Name& name) : _M_name(name)
    {}

    SceneLayer::~SceneLayer()
    {
        if (_M_next)
            delete _M_next;
        _M_next = nullptr;

        if (_M_parent)
        {
            _M_parent->_M_next = nullptr;
            _M_parent          = nullptr;
        }
    }

    SceneLayer& SceneLayer::clear()
    {
        for (PrimitiveComponent* component : _M_components)
        {
            component->_M_layer = nullptr;
        }
        _M_components.clear();
        return *this;
    }

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        for (auto& method : methods_callback)
        {
            (renderer->*method)(viewport, this);
        }

        for (PrimitiveComponent* component : _M_components)
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
        return _M_parent;
    }

    SceneLayer* SceneLayer::next() const
    {
        return _M_next;
    }

    const Name& SceneLayer::name() const
    {
        return _M_name;
    }

    void SceneLayer::destroy()
    {
        if (_M_parent)
        {
            _M_parent->_M_next = _M_next;
        }

        if (_M_next)
        {
            _M_next->_M_parent = _M_parent;
        }

        delete this;
    }

    SceneLayer* SceneLayer::find(const Name& name)
    {
        SceneLayer* current = this;

        while (current)
        {
            if (current->_M_name == name)
                return current;
            current = current->_M_next;
        }

        current = this->_M_parent;

        while (current)
        {
            if (current->_M_name == name)
                return current;
            current = current->_M_parent;
        }

        return nullptr;
    }

    SceneLayer* SceneLayer::create_next(const Name& name)
    {
        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer            = new SceneLayer(name);
        new_layer->_M_parent = this;
        new_layer->_M_next   = _M_next;
        _M_next              = new_layer;
        return new_layer;
    }

    SceneLayer* SceneLayer::create_parent(const Name& name)
    {
        if (!_M_can_create_parent)
            return nullptr;

        SceneLayer* new_layer = find(name);

        if (new_layer)
            return new_layer;

        new_layer            = new SceneLayer(name);
        new_layer->_M_parent = _M_parent;
        new_layer->_M_next   = this;
        _M_parent            = new_layer;
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
        if (!component || component->_M_layer == this)
            return *this;

        if (component->_M_layer != nullptr)
        {
            component->_M_layer->remove_component(component);
        }

        _M_components.insert(component);
        return *this;
    }

    SceneLayer& SceneLayer::remove_component(PrimitiveComponent* component)
    {
        if (!component || component->_M_layer != this)
            return *this;

        _M_components.erase(component);
        component->_M_layer = nullptr;
        return *this;
    }
    class AddPrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* _M_octree;
        PrimitiveComponent* _M_primitive;
        AABB_3Df _M_box;

    public:
        AddPrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : _M_octree(octree), _M_primitive(primitive), _M_box(box)
        {}

        int_t execute() override
        {
            _M_octree->push(_M_box, _M_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    class RemovePrimitiveTask : public ExecutableObject
    {
        Scene::SceneOctree* _M_octree;
        PrimitiveComponent* _M_primitive;
        AABB_3Df _M_box;

    public:
        RemovePrimitiveTask(Scene::SceneOctree* octree, PrimitiveComponent* primitive, const AABB_3Df& box)
            : _M_octree(octree), _M_primitive(primitive), _M_box(box)
        {}

        int_t execute() override
        {
            _M_octree->remove(_M_box, _M_primitive);
            return sizeof(AddPrimitiveTask);
        }
    };

    Scene::Scene()
    {
        _M_root_component = Object::new_instance_named<SceneComponent>("Root");

        _M_root_layer                       = new SceneLayer("Root Layer");
        _M_root_layer->_M_can_create_parent = false;

        _M_clear_layer = _M_root_layer->create_next(SceneLayer::name_clear_render_targets);
        _M_clear_layer->methods_callback.push_back(&SceneRenderer::clear_render_targets);

        _M_base_pass_layer = _M_clear_layer->create_next(SceneLayer::name_base_pass);
        _M_base_pass_layer->methods_callback.push_back(&SceneRenderer::begin_rendering_base_pass);

        _M_lighting_layer = _M_base_pass_layer->create_next(SceneLayer::name_light_pass);
        _M_lighting_layer->methods_callback.push_back(&SceneRenderer::begin_lighting_pass);

        _M_scene_output = _M_lighting_layer->create_next(SceneLayer::name_scene_output_pass);
        _M_scene_output->methods_callback.push_back(&SceneRenderer::begin_scene_output_pass);

        _M_post_process_layer = _M_scene_output->create_next(SceneLayer::name_post_process);
        _M_post_process_layer->methods_callback.push_back(&SceneRenderer::begin_postprocess_pass);
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

        return build_views_internal(renderer, _M_octree.root_node());
    }

    Scene& Scene::add_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<AddPrimitiveTask>(&_M_octree, primitive, primitive->bounding_box());
        return *this;
    }

    Scene& Scene::remove_primitive(PrimitiveComponent* primitive)
    {
        render_thread()->insert_new_task<RemovePrimitiveTask>(&_M_octree, primitive, primitive->bounding_box());
        return *this;
    }


    SceneComponent* Scene::root_component() const
    {
        return _M_root_component.ptr();
    }

    Scene::~Scene()
    {
        delete _M_root_layer;
        _M_root_layer = nullptr;
    }
}// namespace Engine
