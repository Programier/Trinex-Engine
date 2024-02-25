#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>
#include <Engine/octree.hpp>

namespace Engine
{
    class PrimitiveComponent;
    class SceneComponent;
    class SceneRenderer;
    class RenderViewport;

    class ENGINE_EXPORT SceneLayer final
    {
    public:
        static ENGINE_EXPORT const Name name_clear_render_targets;
        static ENGINE_EXPORT const Name name_base_pass;
        static ENGINE_EXPORT const Name name_light_pass;
        static ENGINE_EXPORT const Name name_scene_output_pass;
        static ENGINE_EXPORT const Name name_post_process;

        using FunctionCallback = void (*)(SceneRenderer*, RenderViewport*, SceneLayer*);
        using MethodCallback   = void (SceneRenderer::*)(RenderViewport*, SceneLayer*);

        List<FunctionCallback> function_callbacks;
        List<MethodCallback> methods_callback;

    private:
        Set<PrimitiveComponent*> m_components;

        SceneLayer* m_parent = nullptr;
        SceneLayer* m_next   = nullptr;
        Name m_name;
        bool m_can_create_parent = true;


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

        SceneLayer& add_component(PrimitiveComponent* component);
        SceneLayer& remove_component(PrimitiveComponent* component);

        friend class Scene;
    };

    class ENGINE_EXPORT Scene final
    {
    public:
        using SceneOctree = Octree<PrimitiveComponent*>;

    private:
        SceneLayer* m_root_layer         = nullptr;
        SceneLayer* m_clear_layer        = nullptr;
        SceneLayer* m_base_pass_layer    = nullptr;
        SceneLayer* m_lighting_layer     = nullptr;
        SceneLayer* m_scene_output       = nullptr;
        SceneLayer* m_post_process_layer = nullptr;

        SceneOctree m_octree_render_thread;
        SceneOctree m_octree;
        Pointer<SceneComponent> m_root_component;

        Scene& build_views_internal(SceneRenderer* renderer, SceneOctree::Node* node);

    public:
        Scene();
        Scene& build_views(SceneRenderer* renderer);
        Scene& add_primitive(PrimitiveComponent* primitive);
        Scene& remove_primitive(PrimitiveComponent* primitive);
        SceneComponent* root_component() const;
        const SceneOctree& octree() const;
        ~Scene();


        FORCE_INLINE SceneLayer* root_layer() const
        {
            return m_root_layer;
        }

        FORCE_INLINE SceneLayer* clear_layer() const
        {
            return m_clear_layer;
        }

        FORCE_INLINE SceneLayer* base_pass_layer() const
        {
            return m_base_pass_layer;
        }

        FORCE_INLINE SceneLayer* lighting_layer() const
        {
            return m_lighting_layer;
        }

        FORCE_INLINE SceneLayer* scene_output_layer() const
        {
            return m_scene_output;
        }

        FORCE_INLINE SceneLayer* post_process_layer() const
        {
            return m_post_process_layer;
        }
    };
}// namespace Engine
