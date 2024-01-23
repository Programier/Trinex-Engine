#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;
    class SceneView;


    class ENGINE_EXPORT SceneLayer final
    {
    public:
        using FunctionCallback = void (*)(SceneRenderer*, RenderViewport*, SceneLayer*);
        using MethodCallback   = void (SceneRenderer::*)(RenderViewport*, SceneLayer*);

        List<FunctionCallback> function_callbacks;
        List<MethodCallback> method_callback;


    private:
        SceneLayer* _M_parent = nullptr;
        SceneLayer* _M_next   = nullptr;
        Name _M_name;
        bool _M_can_create_parent = true;


        SceneLayer(const Name& name);
        ~SceneLayer();

    public:
        delete_copy_constructors(SceneLayer);

        SceneLayer& clear();
        SceneLayer& render(SceneRenderer*, RenderViewport*);
        SceneLayer* parent() const;
        SceneLayer* next() const;
        const Name& name() const;

        void destroy();
        SceneLayer* find(const Name& name);
        SceneLayer* create_next(const Name& name);
        SceneLayer* create_parent(const Name& name);


        friend class SceneRenderer;
    };

    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        SceneLayer* _M_root_layer = nullptr;
        class SceneInterface* _M_scene;

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(SceneInterface* scene);
        SceneInterface* scene() const;
        SceneRenderer& render(const SceneView& view);

        FORCE_INLINE SceneLayer* root_layer() const
        {
            return _M_root_layer;
        }

        ~SceneRenderer();
    };

}// namespace Engine
