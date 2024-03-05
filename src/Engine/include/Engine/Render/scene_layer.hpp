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

    class ENGINE_EXPORT SceneLayer final
    {
    public:
        static ENGINE_EXPORT const Name name_clear_render_targets;
        static ENGINE_EXPORT const Name name_base_pass;
        static ENGINE_EXPORT const Name name_gbuffer_to_scene_output;
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
        Set<PrimitiveComponent*> m_components;
        Set<LightComponent*> m_light_components;

        SceneLayer* m_parent = nullptr;
        SceneLayer* m_next   = nullptr;
        Name m_name;
        bool m_can_create_parent = true;


        SceneLayer(const Name& name);
        ~SceneLayer();

    public:
        delete_copy_constructors(SceneLayer);

        SceneLayer& clear();
        SceneLayer& render(SceneRenderer*, RenderTargetBase*);
        SceneLayer* parent() const;
        SceneLayer* next() const;
        const Name& name() const;

        void destroy();
        SceneLayer* find(const Name& name);
        SceneLayer* create_next(const Name& name);
        SceneLayer* create_parent(const Name& name);

        SceneLayer& add_component(PrimitiveComponent* component);
        SceneLayer& remove_component(PrimitiveComponent* component);
        SceneLayer& add_light(LightComponent* component);
        SceneLayer& remove_light(LightComponent* component);

        const Set<PrimitiveComponent*>& primitive_components() const;
        const Set<LightComponent*>& light_components() const;

        friend class Scene;
    };
}// namespace Engine
