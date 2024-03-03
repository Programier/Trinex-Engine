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
    class LightComponent;
    class SceneComponent;
    class SceneRenderer;
    class RenderViewport;
    class RenderTargetBase;
    class SceneLayer;


    class ENGINE_EXPORT Scene final
    {
    public:
        using PrimitiveOctree = Octree<PrimitiveComponent*>;
        using LightOctree     = Octree<LightComponent*>;

    private:
        SceneLayer* m_root_layer         = nullptr;
        SceneLayer* m_clear_layer        = nullptr;
        SceneLayer* m_base_pass_layer    = nullptr;
        SceneLayer* m_lighting_layer     = nullptr;
        SceneLayer* m_scene_output       = nullptr;
        SceneLayer* m_post_process_layer = nullptr;

        PrimitiveOctree m_octree_render_thread;
        PrimitiveOctree m_octree;
        LightOctree m_light_octree_render_thread;
        LightOctree m_light_octree;
        Pointer<SceneComponent> m_root_component;

    public:
        Scene();
        Scene& build_views(SceneRenderer* renderer);
        Scene& add_primitive(PrimitiveComponent* primitive);
        Scene& remove_primitive(PrimitiveComponent* primitive);
        Scene& add_light(LightComponent* light);
        Scene& remove_light(LightComponent* light);
        SceneComponent* root_component() const;
        const PrimitiveOctree& primitive_octree() const;
        const LightOctree& light_octree() const;
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
