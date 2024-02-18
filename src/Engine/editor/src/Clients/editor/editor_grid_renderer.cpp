#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
    static PositionVertexBuffer* axis_vertex_buffer = nullptr;
    static Material* grid_material                  = nullptr;

    static void initialize_resources()
    {
        DefaultResourcesInitializeController().require("Load Editor Package");
        axis_vertex_buffer         = Object::new_instance<PositionVertexBuffer>();
        axis_vertex_buffer->buffer = {
                {-1, 0, 0},
                {1, 0, 0},
                {0, 0, -1},
                {0, 0, 1},
        };

        axis_vertex_buffer->init_resource();

        grid_material = Object::find_object_checked<Material>("Editor::GridMaterial");
    }

    static DefaultResourcesInitializeController on_init(initialize_resources);

    void render_editor_grid(SceneRenderer* renderer, RenderViewport* viewport, SceneLayer* layer)
    {
        if (grid_material && axis_vertex_buffer)
        {
            SceneColorOutput::instance()->rhi_bind();
            grid_material->apply();
            axis_vertex_buffer->rhi_bind(0);
            engine_instance->rhi()->draw(axis_vertex_buffer->buffer.size());
        }
    }
}// namespace Engine
