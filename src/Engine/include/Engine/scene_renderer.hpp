#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;
    class CameraView;


    class ENGINE_EXPORT SceneLayer final
    {
    public:
        static ENGINE_EXPORT const Name name_clear_render_targets;
        static ENGINE_EXPORT const Name name_base_pass;
        static ENGINE_EXPORT const Name name_light_pass;
        static ENGINE_EXPORT const Name name_post_process;
        static ENGINE_EXPORT const Name name_output;

        using FunctionCallback = void (*)(SceneRenderer*, RenderViewport*, SceneLayer*, const CameraView&);
        using MethodCallback   = void (SceneRenderer::*)(RenderViewport*, SceneLayer*, const CameraView&);
        List<FunctionCallback> function_callbacks;
        List<MethodCallback> methods_callback;

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
        SceneLayer& render(SceneRenderer*, RenderViewport*, const CameraView& view);
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
        SceneLayer* _M_root_layer         = nullptr;
        SceneLayer* _M_clear_layer        = nullptr;
        SceneLayer* _M_base_pass_layer    = nullptr;
        SceneLayer* _M_lighting_layer     = nullptr;
        SceneLayer* _M_post_process_layer = nullptr;
        SceneLayer* _M_output_layer       = nullptr;

        class SceneInterface* _M_scene;

        void clear_render_targets(RenderViewport*, SceneLayer*, const CameraView&);

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(SceneInterface* scene);
        SceneInterface* scene() const;
        SceneRenderer& render(const CameraView& view, RenderViewport* viewport);

        FORCE_INLINE SceneLayer* root_layer() const
        {
            return _M_root_layer;
        }

        FORCE_INLINE SceneLayer* clear_layer() const
        {
            return _M_clear_layer;
        }

        FORCE_INLINE SceneLayer* base_pass_layer() const
        {
            return _M_base_pass_layer;
        }

        FORCE_INLINE SceneLayer* lighting_layer() const
        {
            return _M_lighting_layer;
        }

        FORCE_INLINE SceneLayer* post_process_layer() const
        {
            return _M_post_process_layer;
        }

        FORCE_INLINE SceneLayer* output_layer() const
        {
            return _M_output_layer;
        }

        ~SceneRenderer();
    };

}// namespace Engine
