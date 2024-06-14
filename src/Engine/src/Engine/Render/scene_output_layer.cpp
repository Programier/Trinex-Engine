#include <Engine/Render/scene_output_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/octree.hpp>
#include <Engine/scene.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{

    template<typename NodeType>
    static void render_octree_bounding_box(NodeType* node, BatchedLines& lines)
    {
        if (node == nullptr)
            return;

        node->box().write_to_batcher(lines);

        for (byte i = 0; i < 8; i++)
        {
            render_octree_bounding_box(node->child_at(i), lines);
        }
    }

    SceneOutputLayer& SceneOutputLayer::render_light_octree(class Scene* scene)
    {
        render_octree_bounding_box(scene->light_octree().root_node(), lines);
        return *this;
    }

    SceneOutputLayer& SceneOutputLayer::render_primitive_octree(class Scene* scene)
    {
        render_octree_bounding_box(scene->primitive_octree().root_node(), lines);
        return *this;
    }

    SceneOutputLayer& SceneOutputLayer::clear()
    {
        CommandBufferLayer::clear();
        lines.clear();
        triangles.clear();
        return *this;
    }

    SceneOutputLayer& SceneOutputLayer::begin_render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        CommandBufferLayer::begin_render(renderer, viewport);
        return *this;
    }

    SceneOutputLayer& SceneOutputLayer::render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        CommandBufferLayer::render(renderer, viewport);

        if (renderer->scene_view().show_flags().has_all(ShowFlags::LightOctree))
        {
            render_light_octree(renderer->scene);
        }

        if (renderer->scene_view().show_flags().has_all(ShowFlags::PrimitiveOctree))
        {
            render_primitive_octree(renderer->scene);
        }


        lines.render(renderer->scene_view());
        triangles.render(renderer->scene_view());
        return *this;
    }

    SceneOutputLayer& SceneOutputLayer::end_render(SceneRenderer* renderer, RenderViewport* viewport)
    {
        CommandBufferLayer::end_render(renderer, viewport);
        return *this;
    }
}// namespace Engine
