#pragma once
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Engine/Render/batched_lines.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderTargetBase;
    class PrimitiveComponent;
    class LightComponent;

    class ENGINE_EXPORT SceneLayer
    {
    public:
        enum class Type
        {
            Default  = 0,
            Lighting = 1,
            Custom   = 2,
        };

        static ENGINE_EXPORT const Name name_clear_render_targets;
        static ENGINE_EXPORT const Name name_base_pass;
        static ENGINE_EXPORT const Name name_deferred_light_pass;
        static ENGINE_EXPORT const Name name_light_pass;
        static ENGINE_EXPORT const Name name_scene_output_pass;
        static ENGINE_EXPORT const Name name_post_process;

        using FunctionCallback = void (*)(SceneRenderer*, RenderTargetBase*, SceneLayer*);
        using MethodCallback   = void (SceneRenderer::*)(RenderTargetBase*, SceneLayer*);

        BatchedLines lines;
        List<FunctionCallback> begin_render_function_callbacks;
        List<MethodCallback> begin_render_methods_callbacks;
        List<FunctionCallback> end_render_function_callbacks;
        List<MethodCallback> end_render_methods_callbacks;

    private:
        SceneLayer* create_next_internal(const Name& name, SceneLayer* (*allocator)(const Name& name));
        SceneLayer* create_parent_internal(const Name& name, SceneLayer* (*allocator)(const Name& name));

        template<typename LayerType>
        static SceneLayer* static_layer_allocator(const Name& name)
        {
            SceneLayer* layer = new LayerType();
            layer->m_name     = name;
            return layer;
        }

    protected:
        Set<PrimitiveComponent*> m_components;

        SceneLayer* m_parent = nullptr;
        SceneLayer* m_next   = nullptr;
        Name m_name;
        bool m_can_create_parent = true;


        SceneLayer();
        SceneLayer(const Name& name);
        virtual ~SceneLayer();

    public:
        delete_copy_constructors(SceneLayer);

        SceneLayer* parent() const;
        SceneLayer* next() const;
        const Name& name() const;

        void destroy();
        SceneLayer* find(const Name& name);

        template<typename Type = SceneLayer>
        Type* create_next(const Name& name)
        {
            return reinterpret_cast<Type*>(create_next_internal(name, static_layer_allocator<Type>));
        }

        template<typename Type = SceneLayer>
        Type* create_parent(const Name& name)
        {
            return reinterpret_cast<Type*>(create_parent_internal(name, static_layer_allocator<Type>));
        }

        const Set<PrimitiveComponent*>& primitive_components() const;

        virtual Type type() const;
        virtual SceneLayer& clear();
        virtual SceneLayer& begin_render(SceneRenderer* renderer, RenderTargetBase* render_target);
        virtual SceneLayer& render(SceneRenderer*, RenderTargetBase*);
        virtual SceneLayer& end_render(SceneRenderer* renderer, RenderTargetBase* render_target);
        virtual SceneLayer& add_component(PrimitiveComponent* component);
        virtual SceneLayer& remove_component(PrimitiveComponent* component);
        virtual SceneLayer& add_light(LightComponent* component);
        virtual SceneLayer& remove_light(LightComponent* component);

        friend class SceneRenderer;
    };

    class ENGINE_EXPORT LightingSceneLayer : public SceneLayer
    {
    protected:
        Set<LightComponent*> m_light_components;

    public:
        SceneLayer::Type type() const override;
        LightingSceneLayer& clear() override;
        LightingSceneLayer& render(SceneRenderer*, RenderTargetBase*) override;
        LightingSceneLayer& add_light(LightComponent* component) override;
        LightingSceneLayer& remove_light(LightComponent* component) override;
        const Set<LightComponent*>& light_components() const;
        ~LightingSceneLayer();

        friend class SceneLayer;
    };
}// namespace Engine
