#include <Core/default_resources.hpp>
#include <Core/engine.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/render_thread.hpp>
#include <Engine/Render/batched_lines.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    BatchedLines::BatchedLines()
    {
        m_position_buffer = Object::new_instance<EngineResource<PositionVertexBuffer>>();
        m_color_buffer    = Object::new_instance<EngineResource<ColorVertexBuffer>>();
    }

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

    BatchedLines& BatchedLines::clear()
    {
        m_position_buffer->buffer.clear();
        m_color_buffer->buffer.clear();
        return *this;
    }

    BatchedLines& BatchedLines::render(const class SceneView& view)
    {
        if (m_allocated_size < m_position_buffer->buffer.size())
        {
            m_position_buffer->rhi_create();
            m_color_buffer->rhi_create();
            m_allocated_size = m_position_buffer->buffer.size();
        }
        else if(m_position_buffer->buffer.size() > 0)
        {
            m_position_buffer->rhi_update(0, m_allocated_size * sizeof(PositionVertexBuffer::ElementType),
                                          reinterpret_cast<const byte*>(m_position_buffer->data()));
            m_color_buffer->rhi_update(0, m_allocated_size * sizeof(ColorVertexBuffer::ElementType),
                                       reinterpret_cast<const byte*>(m_color_buffer->data()));
        }

        if (m_position_buffer->buffer.size() == 0)
            return *this;

        auto pass          = RenderTargetBase::current_target()->render_pass->type();
        Material* material = nullptr;

        if (pass == RenderPassType::GBuffer)
        {
            material = DefaultResources::gbuffer_lines_material;
        }
        else if (pass == RenderPassType::OneAttachentOutput)
        {
            material = DefaultResources::scene_output_lines_material;
        }

        if (material)
        {
            RHI* rhi = engine_instance->rhi();
#if TRINEX_DEBUG_BUILD
            rhi->push_debug_stage("Lines Rendering");
#endif
            material->apply();
            m_position_buffer->rhi_bind(0);
            m_color_buffer->rhi_bind(1);

            rhi->draw(m_position_buffer->buffer.size());

#if TRINEX_DEBUG_BUILD
            rhi->pop_debug_stage();
#endif
        }

        return *this;
    }

    BatchedLines::~BatchedLines()
    {
        delete m_position_buffer;
        delete m_color_buffer;
    }
}// namespace Engine
