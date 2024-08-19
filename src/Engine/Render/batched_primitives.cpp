#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/threading.hpp>
#include <Engine/Render/batched_primitives.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{

	template<typename T>
	static void submit_vertex_buffer(T* buffer, size_t& current_size)
	{
		if (buffer->buffer.size() < current_size)
		{
			buffer->rhi_update(0, buffer->buffer.size(), buffer->data());
		}
		else
		{
			buffer->rhi_create();
			current_size = buffer->buffer.size();
		}
	}

	BatchedPrimitive::BatchedPrimitive()
	{
		m_position_buffer = Object::new_instance<PositionDynamicVertexBuffer>();
		m_color_buffer	  = Object::new_instance<ColorDynamicVertexBuffer>();
	}

	BatchedPrimitive& BatchedPrimitive::clear()
	{
		m_position_buffer->buffer.clear();
		m_color_buffer->buffer.clear();
		return *this;
	}

	bool BatchedPrimitive::begin_render()
	{
		if (m_position_buffer->buffer.size() == 0)
			return false;


		submit_vertex_buffer(m_position_buffer.ptr(), m_position_buffer_size);
		submit_vertex_buffer(m_color_buffer.ptr(), m_color_buffer_size);
		return true;
	}

	BatchedPrimitive::~BatchedPrimitive()
	{}

	//// Lines

	BatchedLines::BatchedLines() : m_vertex_count(0)
	{
		m_lines = Object::new_instance<LinesVertexBuffer>();
	}

	BatchedLines& BatchedLines::add_line(const Vertex& point1, const Vertex& point2)
	{
		m_lines->buffer.push_back(point1);
		m_lines->buffer.push_back(point2);
		return *this;
	}

	BatchedLines& BatchedLines::render(const class SceneView& view)
	{
		if (m_lines->buffer.size() == 0)
			return *this;

		submit_vertex_buffer(m_lines.ptr(), m_vertex_count);

		Material* material = DefaultResources::Materials::batched_lines;

		if (material)
		{
#if TRINEX_DEBUG_BUILD
			rhi->push_debug_stage("Lines Rendering");
#endif
			material->apply();
			m_lines->rhi_bind(0);

			rhi->draw(m_lines->buffer.size(), 0);

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		return *this;
	}

	BatchedLines& BatchedLines::clear()
	{
		m_lines->buffer.clear();
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

	BatchedTriangles& BatchedTriangles::override_triangle(Index index, const Vector3D& point1, const Vector3D& point2,
														  const Vector3D& point3, ByteColor color1, ByteColor color2,
														  ByteColor color3)
	{
		index *= 3;
		if (m_position_buffer->buffer.size() <= index)
			return add_triangle(point1, point2, point3, color1, color2, color3);

		m_position_buffer->buffer[index]	 = point1;
		m_position_buffer->buffer[index + 1] = point2;
		m_position_buffer->buffer[index + 2] = point2;

		m_color_buffer->buffer[index]	  = color1;
		m_color_buffer->buffer[index + 1] = color2;
		m_color_buffer->buffer[index + 2] = color3;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::render(const class SceneView& view)
	{
		if (!begin_render())
			return *this;

		Material* material = DefaultResources::Materials::batched_triangles;

		if (material)
		{
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
