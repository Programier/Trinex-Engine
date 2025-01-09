#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/etl/engine_resource.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/threading.hpp>
#include <Engine/Render/batched_primitives.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	template<typename T>
	static void submit_vertex_buffer(T* buffer)
	{
		auto cpu_buffer = buffer->buffer();

		if (cpu_buffer->size() * sizeof(typename T::ElementType) < buffer->size())
		{
			buffer->rhi_update(0, cpu_buffer->size() * sizeof(typename T::ElementType), buffer->data());
		}
		else
		{
			buffer->rhi_init();
		}
	}

	//// Lines

	BatchedLines::BatchedLines()
	{
		m_lines = Object::new_instance<LinesVertexBuffer>();
		m_lines->allocate_data(true);
	}

	BatchedLines& BatchedLines::add_line(const Vertex& point1, const Vertex& point2)
	{
		auto cpu_buffer = m_lines->buffer();
		cpu_buffer->push_back(point1);
		cpu_buffer->push_back(point2);
		return *this;
	}

	BatchedLines& BatchedLines::render(const class SceneView& view)
	{
		if (m_lines->buffer()->size() == 0)
			return *this;

		submit_vertex_buffer(m_lines.ptr());

		Material* material = DefaultResources::Materials::batched_lines;

		if (material)
		{
#if TRINEX_DEBUG_BUILD
			rhi->push_debug_stage("Lines Rendering");
#endif
			material->apply();
			m_lines->rhi_bind(0);

			rhi->draw(m_lines->buffer()->size(), 0);

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		return *this;
	}

	BatchedLines& BatchedLines::clear()
	{
		m_lines->buffer()->clear();
		return *this;
	}


	// TRIANGLES

	BatchedTriangles::BatchedTriangles()
	{
		m_position_buffer = Object::new_instance<PositionDynamicVertexBuffer>();
		m_color_buffer    = Object::new_instance<ColorDynamicVertexBuffer>();

		m_position_buffer->allocate_data(true);
		m_color_buffer->allocate_data(true);
	}

	BatchedTriangles& BatchedTriangles::clear()
	{
		m_position_buffer->buffer()->clear();
		m_color_buffer->buffer()->clear();
		return *this;
	}

	BatchedTriangles& BatchedTriangles::add_triangle(const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
	                                                 ByteColor color1, ByteColor color2, ByteColor color3)
	{
		auto pos_cpu_buffer   = m_position_buffer->buffer();
		auto color_cpu_buffer = m_color_buffer->buffer();

		pos_cpu_buffer->push_back(point1);
		pos_cpu_buffer->push_back(point2);
		pos_cpu_buffer->push_back(point3);

		color_cpu_buffer->push_back(color1);
		color_cpu_buffer->push_back(color2);
		color_cpu_buffer->push_back(color3);
		return *this;
	}

	BatchedTriangles& BatchedTriangles::override_triangle(Index index, const Vector3D& point1, const Vector3D& point2,
	                                                      const Vector3D& point3, ByteColor color1, ByteColor color2,
	                                                      ByteColor color3)
	{
		auto& pos_cpu_buffer   = *m_position_buffer->buffer();
		auto& color_cpu_buffer = *m_color_buffer->buffer();

		index *= 3;
		if (pos_cpu_buffer.size() <= index)
			return add_triangle(point1, point2, point3, color1, color2, color3);

		pos_cpu_buffer[index]     = point1;
		pos_cpu_buffer[index + 1] = point2;
		pos_cpu_buffer[index + 2] = point2;

		color_cpu_buffer[index]     = color1;
		color_cpu_buffer[index + 1] = color2;
		color_cpu_buffer[index + 2] = color3;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::render(const class SceneView& view)
	{
		if (m_position_buffer->buffer()->size() == 0)
			return *this;

		submit_vertex_buffer(m_position_buffer.ptr());
		submit_vertex_buffer(m_color_buffer.ptr());

		Material* material = DefaultResources::Materials::batched_triangles;

		if (material)
		{
#if TRINEX_DEBUG_BUILD
			rhi->push_debug_stage("Triangles Rendering");
#endif
			material->apply();
			m_position_buffer->rhi_bind(0);
			m_color_buffer->rhi_bind(1);

			rhi->draw(m_position_buffer->buffer()->size(), 0);

#if TRINEX_DEBUG_BUILD
			rhi->pop_debug_stage();
#endif
		}

		return *this;
	}
}// namespace Engine
