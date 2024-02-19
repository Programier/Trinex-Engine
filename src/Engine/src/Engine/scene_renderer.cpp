#include <Core/engine.hpp>
#include <Engine/scene.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>


namespace Engine
{
    const Name SceneLayer::name_clear_render_targets = "Clear Render Targets";
    const Name SceneLayer::name_base_pass            = "Base Pass";
    const Name SceneLayer::name_light_pass           = "Light Pass";
    const Name SceneLayer::name_post_process         = "Post Process";
    const Name SceneLayer::name_output               = "Output Pass";

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

    SceneLayer& SceneLayer::render(SceneRenderer* renderer, RenderViewport* viewport, const CameraView& view)
    {
        for (auto& method : methods_callback)
        {
            (renderer->*method)(viewport, this, view);
        }

        for (auto& func : function_callbacks)
        {
            func(renderer, viewport, this, view);
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

    void SceneRenderer::clear_render_targets(RenderViewport*, SceneLayer*, const CameraView&)
    {
        GBuffer::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearGBuffer));
        SceneColorOutput::instance()->rhi_bind(RenderPass::load_render_pass(RenderPassType::ClearSceneOutput));
    }

    SceneRenderer::SceneRenderer() : _M_scene(nullptr)
    {
        _M_root_layer                       = new SceneLayer("Root Layer");
        _M_root_layer->_M_can_create_parent = false;

        _M_clear_layer        = _M_root_layer->create_next(SceneLayer::name_clear_render_targets);
        _M_base_pass_layer    = _M_clear_layer->create_next(SceneLayer::name_base_pass);
        _M_lighting_layer     = _M_base_pass_layer->create_next(SceneLayer::name_light_pass);
        _M_post_process_layer = _M_lighting_layer->create_next(SceneLayer::name_post_process);
        _M_output_layer       = _M_post_process_layer->create_next(SceneLayer::name_output);

        _M_clear_layer->methods_callback.push_back(&SceneRenderer::clear_render_targets);
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

    SceneRenderer& SceneRenderer::render(const CameraView& view, RenderViewport* viewport)
    {
#if TRINEX_DEBUG_BUILD
        auto rhi = engine_instance->rhi();
#endif
        for (auto layer = root_layer(); layer; layer = layer->next())
        {
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage(layer->_M_name.c_str());
#endif
            layer->render(this, viewport, view);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }
        return *this;
    }

    SceneRenderer::~SceneRenderer()
    {
        delete _M_root_layer;
    }
}// namespace Engine
