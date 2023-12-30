#include <Engine/scene_renderer.hpp>


namespace Engine
{

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

    SceneLayer* SceneLayer::create_next()
    {
        SceneLayer* new_layer = new SceneLayer();
        new_layer->_M_parent  = this;
        new_layer->_M_next    = _M_next;
        _M_next               = new_layer;
        return new_layer;
    }

    SceneLayer* SceneLayer::create_parent()
    {
        if (!_M_can_create_parent)
            return nullptr;

        SceneLayer* new_layer = new SceneLayer();
        new_layer->_M_parent  = _M_parent;
        new_layer->_M_next    = this;
        _M_parent             = new_layer;
        return new_layer;
    }

    SceneRenderer::SceneRenderer()
    {
        _M_root_layer                       = new SceneLayer();
        _M_root_layer->_M_can_create_parent = false;
    }

    SceneRenderer::~SceneRenderer()
    {}
}// namespace Engine
