#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/render_thread.hpp>
#include <Engine/Render/batched_primitives.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{

    BatchedPrimitive::BatchedPrimitive()
    {
        m_position_buffer = Object::new_instance<EngineResource<PositionDynamicVertexBuffer>>();
        m_color_buffer    = Object::new_instance<EngineResource<ColorDynamicVertexBuffer>>();
    }

    BatchedPrimitive& BatchedPrimitive::clear()
    {
        m_position_buffer->buffer.clear();
        m_color_buffer->buffer.clear();
        return *this;
    }

    bool BatchedPrimitive::begin_render()
    {
        RenderTargetBase* rt = RenderTargetBase::current_target();
        if(rt == nullptr)
            return false;

        auto pass = rt->render_pass->type();

        // Render pass always must be SceneColor!
        if (pass != RenderPassType::SceneColor)
            return false;

        if (m_position_buffer->buffer.size() == 0)
            return false;

        m_position_buffer->rhi_submit_changes();
        m_color_buffer->rhi_submit_changes();
        return true;
    }

    BatchedPrimitive::~BatchedPrimitive()
    {
        GarbageCollector::destroy(m_position_buffer);
        GarbageCollector::destroy(m_color_buffer);
    }


    //// Lines

    BatchedLines& BatchedLines::add_line(const Vector3D& point1, const Vector3D& point2, ByteColor color1, ByteColor color2)
    {
        m_position_buffer->buffer.push_back(point1);
        m_position_buffer->buffer.push_back(point2);

        m_color_buffer->buffer.push_back(color1);
        m_color_buffer->buffer.push_back(color2);
        return *this;
    }

    BatchedLines& BatchedLines::override_line(Index index, const Vector3D& point1, const Vector3D& point2, ByteColor color1,
                                              ByteColor color2)
    {
        index *= 2;
        if (m_position_buffer->buffer.size() <= index)
            return add_line(point1, point2, color1, color2);

        m_position_buffer->buffer[index]     = point1;
        m_position_buffer->buffer[index + 1] = point2;

        m_color_buffer->buffer[index]     = color1;
        m_color_buffer->buffer[index + 1] = color2;
        return *this;
    }

    BatchedLines& BatchedLines::render(const class SceneView& view)
    {
        if (!begin_render())
            return *this;

        Material* material = DefaultResources::batched_lines_material;

        if (material)
        {
            RHI* rhi = engine_instance->rhi();
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage("Lines Rendering");
#endif
            material->apply();
            m_position_buffer->rhi_bind(0);
            m_color_buffer->rhi_bind(1);

            rhi->draw(m_position_buffer->buffer.size(), 0);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        return *this;
    }

    BatchedTriangles& BatchedTriangles::add_triangle(const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
                                                     ByteColor color1, ByteColor color2, ByteColor color3)
    {
        m_position_buffer->buffer.push_back(point1);
        m_position_buffer->buffer.push_back(point2);
        m_position_buffer->buffer.push_back(point3);

        m_color_buffer->buffer.push_back(color1);
        m_color_buffer->buffer.push_back(color2);
        m_color_buffer->buffer.push_back(color3);
        return *this;
    }

    BatchedTriangles& BatchedTriangles::override_line(Index index, const Vector3D& point1, const Vector3D& point2,
                                                      const Vector3D& point3, ByteColor color1, ByteColor color2,
                                                      ByteColor color3)
    {
        index *= 3;
        if (m_position_buffer->buffer.size() <= index)
            return add_triangle(point1, point2, point3, color1, color2, color3);

        m_position_buffer->buffer[index]     = point1;
        m_position_buffer->buffer[index + 1] = point2;
        m_position_buffer->buffer[index + 2] = point2;

        m_color_buffer->buffer[index]     = color1;
        m_color_buffer->buffer[index + 1] = color2;
        m_color_buffer->buffer[index + 2] = color3;
        return *this;
    }

    BatchedTriangles& BatchedTriangles::render(const class SceneView& view)
    {
        if (!begin_render())
            return *this;

        Material* material = DefaultResources::batched_triangles_material;

        if (material)
        {
            RHI* rhi = engine_instance->rhi();
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage("Triangles Rendering");
#endif
            material->apply();
            m_position_buffer->rhi_bind(0);
            m_color_buffer->rhi_bind(1);

            rhi->draw(m_position_buffer->buffer.size(), 0);

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        return *this;
    }
}// namespace Engine
