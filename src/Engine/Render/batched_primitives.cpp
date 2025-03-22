#include <Engine/Render/batched_primitives.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/rhi.hpp>
#include <cstring>


namespace Engine
{
	template<typename T>
	static void grow_buffer(T& dst)
	{
		T old    = std::move(dst);
		auto ptr = dst.allocate_data(RHIBufferType::Dynamic, old.size() * 2);
		std::memcpy(ptr, old.data(), old.size());
	}

	static inline void submit_vertex_buffer(VertexBufferBase& buffer, size_t vertices)
	{
		if (buffer.rhi_vertex_buffer() == nullptr)
			buffer.init(true);
		else
			buffer.rhi_update(vertices * buffer.stride());
	}

	//// Lines

	BatchedLines::BatchedLines()
	{
		m_vtx_buffer.allocate_data(RHIBufferType::Dynamic, 64);
	}

	BatchedLines& BatchedLines::add_line(const Vertex& point1, const Vertex& point2)
	{
		if (m_vtx_count + 2 > m_vtx_buffer.size())
			grow_buffer(m_vtx_buffer);

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

		submit_vertex_buffer(m_vtx_buffer, m_vtx_count);

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
	{
		m_position_buffer.allocate_data(RHIBufferType::Dynamic, 64);
		m_color_buffer.allocate_data(RHIBufferType::Dynamic, 64);
	}

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
			grow_buffer(m_position_buffer);
			grow_buffer(m_color_buffer);
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

		submit_vertex_buffer(m_position_buffer, m_vtx_count);
		submit_vertex_buffer(m_color_buffer, m_vtx_count);

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
