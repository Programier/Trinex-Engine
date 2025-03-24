#include <Engine/Render/batched_primitives.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <cstring>


namespace Engine
{
	//// Lines

	BatchedLines::BatchedLines() : m_vtx_buffer(RHIBufferType::Dynamic, 64, nullptr, true) {}

	BatchedLines& BatchedLines::add_line(const Vertex& point1, const Vertex& point2)
	{
		if (m_vtx_count + 2 > m_vtx_buffer.size())
			m_vtx_buffer.grow();

		auto dst = m_vtx_buffer.data() + m_vtx_count;
		new (dst) Vertex(point1);
		new (dst + 1) Vertex(point2);
		m_vtx_count += 2;
		return *this;
	}

	BatchedLines& BatchedLines::render(class RenderPass* pass)
	{
		if (m_vtx_count == 0)
			return *this;

		m_vtx_buffer.rhi_update(m_vtx_count * m_vtx_buffer.stride());

#if TRINEX_DEBUG_BUILD
		rhi->push_debug_stage("Lines Rendering");
#endif

		Pipelines::BatchedLines::instance()->apply(pass->scene_renderer());
		m_vtx_buffer.rhi_bind(0);

		rhi->draw(m_vtx_count, 0);

#if TRINEX_DEBUG_BUILD
		rhi->pop_debug_stage();
#endif
		return *this;
	}

	BatchedLines& BatchedLines::clear()
	{
		m_vtx_count = 0;
		return *this;
	}

	// TRIANGLES

	BatchedTriangles::BatchedTriangles()
		: m_position_buffer(RHIBufferType::Dynamic, 64, nullptr, true), m_color_buffer(RHIBufferType::Dynamic, 64, nullptr, true)
	{}

	BatchedTriangles& BatchedTriangles::clear()
	{
		m_vtx_count = 0;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::add_triangle(const Vector3f& point1, const Vector3f& point2, const Vector3f& point3,
	                                                 ByteColor color1, ByteColor color2, ByteColor color3)
	{
		if (m_vtx_count + 3 > m_position_buffer.size())
		{
			m_position_buffer.grow();
			m_color_buffer.grow();
		}

		auto pos_dst = m_position_buffer.data() + m_vtx_count;
		auto col_dst = m_color_buffer.data() + m_vtx_count;

		new (pos_dst) Vector3f(point1);
		new (pos_dst + 1) Vector3f(point2);
		new (pos_dst + 2) Vector3f(point3);

		new (col_dst) ByteColor(color1);
		new (col_dst + 1) ByteColor(color2);
		new (col_dst + 2) ByteColor(color3);

		m_vtx_count += 3;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::render(class RenderPass* pass)
	{
		if (m_vtx_count == 0)
			return *this;

		m_position_buffer.rhi_update(m_vtx_count * m_position_buffer.stride());
		m_color_buffer.rhi_update(m_vtx_count * m_color_buffer.stride());

#if TRINEX_DEBUG_BUILD
		rhi->push_debug_stage("Triangles Rendering");
#endif

		Pipelines::BatchedTriangles::instance()->rhi_bind();
		m_position_buffer.rhi_bind(0);
		m_color_buffer.rhi_bind(1);

		rhi->draw(m_vtx_count, 0);

#if TRINEX_DEBUG_BUILD
		rhi->pop_debug_stage();
#endif

		return *this;
	}
}// namespace Engine
