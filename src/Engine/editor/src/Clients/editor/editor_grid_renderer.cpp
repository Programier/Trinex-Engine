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

    static int lines_per_axis = 100;

    static PositionVertexBuffer* x_axis_vertex_buffer = nullptr;
    static PositionVertexBuffer* y_axis_vertex_buffer = nullptr;
    static PositionVertexBuffer* grid_vertex_buffer   = nullptr;

    static Material* axis_material = nullptr;
    static Material* grid_material = nullptr;

    static void initialize_resources()
    {
        DefaultResourcesInitializeController().require("Load Editor Package");
        x_axis_vertex_buffer = Object::new_instance<PositionVertexBuffer>();
        y_axis_vertex_buffer = Object::new_instance<PositionVertexBuffer>();
        grid_vertex_buffer   = Object::new_instance<PositionVertexBuffer>();

        x_axis_vertex_buffer->buffer = {{-lines_per_axis, 0, 0}, {lines_per_axis, 0, 0}};
        y_axis_vertex_buffer->buffer = {{0, 0, -lines_per_axis}, {0, 0, lines_per_axis}};

        for (int i = -lines_per_axis; i <= lines_per_axis; i++)
        {
            if (i != 0)
            {
                grid_vertex_buffer->buffer.push_back({-lines_per_axis, 0, i});
                grid_vertex_buffer->buffer.push_back({lines_per_axis, 0, i});

                grid_vertex_buffer->buffer.push_back({i, 0, -lines_per_axis});
                grid_vertex_buffer->buffer.push_back({i, 0, lines_per_axis});

            }
        }

        x_axis_vertex_buffer->init_resource();
        y_axis_vertex_buffer->init_resource();
        grid_vertex_buffer->init_resource();

        axis_material = Object::find_object_checked<Material>("Editor::AxisMaterial");
        grid_material = Object::find_object_checked<Material>("Editor::GridMaterial");
    }

    static DefaultResourcesInitializeController on_init(initialize_resources);

    void render_editor_grid(SceneRenderer* renderer, RenderViewport* viewport, SceneLayer* layer, const CameraView& view)
    {
        static Name color = "color";

        auto rhi = engine_instance->rhi();

        if (axis_material)
        {

            SceneColorOutput::instance()->rhi_bind();
            MaterialParameter* param = axis_material->find_parameter(color);

            (*param->get<Vector3D>()) = {1, 0, 0};
            axis_material->apply();
            x_axis_vertex_buffer->rhi_bind(0);
            rhi->draw(x_axis_vertex_buffer->buffer.size());

            (*param->get<Vector3D>()) = {0, 1, 0};
            axis_material->apply();
            y_axis_vertex_buffer->rhi_bind(0);
            rhi->draw(x_axis_vertex_buffer->buffer.size());
        }

        if (grid_material)
        {
            grid_material->apply();
            grid_vertex_buffer->rhi_bind(0);
            rhi->draw(grid_vertex_buffer->buffer.size());
        }
    }
}// namespace Engine
