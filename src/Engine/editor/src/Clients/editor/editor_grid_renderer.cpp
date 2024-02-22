#include <Core/engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

#include <Graphics/mesh.hpp>

namespace Engine
{
    static int lines_per_axis     = 150;
    static const float scale_step = 8.f;

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
            grid_vertex_buffer->buffer.push_back({-lines_per_axis, 0, i});
            grid_vertex_buffer->buffer.push_back({lines_per_axis, 0, i});

            grid_vertex_buffer->buffer.push_back({i, 0, -lines_per_axis});
            grid_vertex_buffer->buffer.push_back({i, 0, lines_per_axis});
        }

        x_axis_vertex_buffer->init_resource();
        y_axis_vertex_buffer->init_resource();
        grid_vertex_buffer->init_resource();

        axis_material = Object::find_object_checked<Material>("Editor::AxisMaterial");
        grid_material = Object::find_object_checked<Material>("Editor::GridMaterial");
    }

    static DefaultResourcesInitializeController on_init(initialize_resources);

    void render_editor_grid(SceneRenderer* renderer, RenderViewport* viewport, SceneLayer* layer)
    {
        static Name name_color  = "color";
        static Name name_scale  = "scale";
        static Name name_offset = "offset";

        GBuffer::instance()->rhi_bind();

        auto rhi = engine_instance->rhi();

        const CameraView& view = renderer->camera_view();

        float camera_height = glm::abs(view.location.y);
        float scale         = 1.f;
        float next_scale    = scale_step;

        float lower_height = 0.f;
        float upper_height = glm::tan(glm::radians(view.fov)) * scale_step;

        while (upper_height < camera_height)
        {
            lower_height = upper_height;
            upper_height *= scale_step;

            scale = next_scale;
            next_scale *= scale_step;
        }

        float alpha = 1.f - ((camera_height - lower_height) / upper_height);

        if (grid_material)
        {
            float local_scale = scale;
            for (int i = 0; i < 2; i++)
            {
                if (MaterialParameter* param = grid_material->find_parameter(name_scale))
                {
                    (*param->get<float>()) = local_scale;
                }

                if (MaterialParameter* param = grid_material->find_parameter(name_color))
                {
                    (*param->get<Vector4D>()).w = alpha;
                }

                grid_material->apply();
                grid_vertex_buffer->rhi_bind(0);
                rhi->draw(grid_vertex_buffer->buffer.size());

                alpha = 1.f - alpha;
                local_scale = next_scale;
            }
        }

        if (axis_material)
        {
            scale = glm::mix(next_scale, scale, alpha);

            MaterialParameter* color_param  = axis_material->find_parameter(name_color);
            MaterialParameter* offset_param = axis_material->find_parameter(name_offset);
            MaterialParameter* scale_param = axis_material->find_parameter(name_scale);

            if(scale_param)
            {
                (*scale_param->get<float>()) = scale;
            }

            if (color_param)
            {
                (*color_param->get<Vector3D>()) = {1, 0, 0};
            }

            if (offset_param)
            {
                (*offset_param->get<Vector3D>()) = Vector3D(view.location.x, 0, 0);
            }

            axis_material->apply();
            x_axis_vertex_buffer->rhi_bind(0);
            rhi->draw(x_axis_vertex_buffer->buffer.size());

            if (color_param)
            {
                (*color_param->get<Vector3D>()) = {0, 1, 0};
            }

            if (offset_param)
            {
                (*offset_param->get<Vector3D>()) = Vector3D(0, 0, view.location.z);
            }

            axis_material->apply();
            y_axis_vertex_buffer->rhi_bind(0);
            rhi->draw(x_axis_vertex_buffer->buffer.size());
        }
    }
}// namespace Engine
