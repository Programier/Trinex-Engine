#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>


namespace Engine
{

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
        return *this;
    }

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        for (auto& func : function_callbacks)
        {
            func(renderer, viewport, this);
        }

        for (auto& method : method_callback)
        {
            (renderer->*method)(viewport, this);
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

    SceneRenderer::SceneRenderer() : _M_scene(nullptr)
    {
        _M_root_layer                       = new SceneLayer("Root Layer");
        _M_root_layer->_M_can_create_parent = false;
    }

    SceneRenderer& SceneRenderer::scene(SceneInterface* scene)
    {
        _M_scene = scene;
        return *this;
    }

    SceneInterface* SceneRenderer::scene() const
    {
        return _M_scene;
    }

    SceneRenderer& SceneRenderer::render(const SceneView& view)
    {
        return *this;
    }

    SceneRenderer::~SceneRenderer()
    {
        delete _M_root_layer;
    }
}// namespace Engine
