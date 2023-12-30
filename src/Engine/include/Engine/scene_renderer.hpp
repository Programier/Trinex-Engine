#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;


    class ENGINE_EXPORT SceneLayer final
    {
    public:
        using FunctionCallback = void (*)(SceneRenderer*, RenderViewport*, SceneLayer*);
        using MethodCallback   = void (SceneRenderer::*)(RenderViewport*, SceneLayer*);

        List<FunctionCallback> function_callbacks;
        List<MethodCallback> method_callback;


    private:
        SceneLayer* _M_parent     = nullptr;
        SceneLayer* _M_next       = nullptr;
        bool _M_can_create_parent = true;

    public:
        SceneLayer& clear();
        SceneLayer& render(SceneRenderer*, RenderViewport*);
        SceneLayer* parent() const;
        SceneLayer* next() const;

        void destroy();
        SceneLayer* create_next();
        SceneLayer* create_parent();


        friend class SceneRenderer;
    };

    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        SceneLayer* _M_root_layer = nullptr;

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        FORCE_INLINE SceneLayer* root_layer() const
        {
            return _M_root_layer;
        }

        ~SceneRenderer();
    };

}// namespace Engine
