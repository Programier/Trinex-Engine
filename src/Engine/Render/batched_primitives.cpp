#include <Core/etl/allocator.hpp>
#include <Engine/Render/batched_primitives.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/renderer.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/context.hpp>
#include <RHI/rhi.hpp>


namespace Engine
{
	static constexpr size_t s_line_vtx_per_node = 1024;
	static constexpr auto s_vtx_buffer_flags =
	        RHIBufferCreateFlags::VertexBuffer | RHIBufferCreateFlags::TransferDst | RHIBufferCreateFlags::Dynamic;

	BatchedLines::Node::Node() : next(nullptr), vertices(FrameAllocator<Vertex>::allocate(s_line_vtx_per_node)), vtx_count(0) {}

	BatchedLines::BatchedLines() {}

	BatchedLines::Node* BatchedLines::find_node()
	{
		if (m_last == nullptr)
		{
			m_first = m_last = new (FrameAllocator<Node>::allocate(1)) Node();
		}
		else if (m_last->vtx_count == 1024)
		{
			m_last->next = new (FrameAllocator<Node>::allocate(1)) Node();
			m_last       = m_last->next;
		}

		return m_last;
	}

	BatchedLines& BatchedLines::add_line(const Vertex& point1, const Vertex& point2)
	{
		auto node = find_node();

		auto dst = node->vertices + node->vtx_count;
		new (dst) Vertex(point1);
		new (dst + 1) Vertex(point2);
		node->vtx_count += 2;
		return *this;
	}

	BatchedLines& BatchedLines::add_circle(const Vector3f& position, const Vector3f& normal, float radius, const Color& color,
	                                       uint_t segments, float thickness)
	{
		if (segments == 0)
			segments = 72;

		const float step = 2.0f * glm::pi<float>() / static_cast<float>(segments);

		Vector3f n       = glm::normalize(normal);
		Vector3f tangent = std::abs(n.y) < 0.99f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0);

		Vector3f bitangent  = glm::normalize(glm::cross(n, tangent));
		tangent             = glm::normalize(glm::cross(bitangent, n));
		Vector3f prev_point = position + radius * tangent;

		for (uint_t i = 1; i <= segments; ++i)
		{
			float angle         = step * i;
			Vector3f next_point = position + radius * (glm::cos(angle) * tangent + glm::sin(angle) * bitangent);
			add_line(Vertex(prev_point, color, thickness), Vertex(next_point, color, thickness));
			prev_point = next_point;
		}
		return *this;
	}

	BatchedLines& BatchedLines::add_sphere(const Vector3f& position, float radius, const Color& color, uint_t segments,
	                                       float thickness)
	{
		add_circle(position, {1.f, 0.f, 0.f}, radius, color, segments, thickness);
		add_circle(position, {0.f, 1.f, 0.f}, radius, color, segments, thickness);
		add_circle(position, {0.f, 0.f, 1.f}, radius, color, segments, thickness);
		return *this;
	}

	BatchedLines& BatchedLines::add_arrow(const Vector3f& position, const Vector3f& direction, const Color& color,
	                                      float thickness)
	{
		float length = glm::length(direction);
		if (length < 1e-5f)
			return *this;

		Vector3f tip = position + direction;
		add_line(Vertex(position, color, thickness), Vertex(tip, color, thickness));

		Vector3f dir   = glm::normalize(direction);
		Vector3f up    = std::abs(dir.y) < 0.99f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0);
		Vector3f right = glm::normalize(glm::cross(up, dir));
		up             = glm::normalize(glm::cross(dir, right));

		const float head_length = length * 0.1f;
		const float head_width  = head_length * 0.5f;

		Vector3f back = tip - dir * head_length;

		Vector3f head_left  = back + right * head_width;
		Vector3f head_right = back - right * head_width;
		Vector3f head_up    = back + up * head_width;
		Vector3f head_down  = back - up * head_width;

		add_line(Vertex(tip, color, thickness), Vertex(head_left, color, thickness));
		add_line(Vertex(tip, color, thickness), Vertex(head_right, color, thickness));
		add_line(Vertex(tip, color, thickness), Vertex(head_up, color, thickness));
		add_line(Vertex(tip, color, thickness), Vertex(head_down, color, thickness));
		return *this;
	}

	BatchedLines& BatchedLines::add_cone(const Vector3f& position, const Vector3f& direction, float radius, const Color& color,
	                                     uint_t segments, float thickness)
	{
		Vector3f dir = glm::normalize(direction);
		Vector3f tip = position + direction;

		Vector3f tangent   = std::abs(dir.y) < 0.99f ? Vector3f(0, 1, 0) : Vector3f(1, 0, 0);
		Vector3f bitangent = glm::normalize(glm::cross(dir, tangent));
		tangent            = glm::normalize(glm::cross(bitangent, dir));

		add_line(Vertex(position + radius * tangent, color, thickness), Vertex(tip, color, thickness));
		add_line(Vertex(position + radius * bitangent, color, thickness), Vertex(tip, color, thickness));
		add_line(Vertex(position - radius * tangent, color, thickness), Vertex(tip, color, thickness));
		add_line(Vertex(position - radius * bitangent, color, thickness), Vertex(tip, color, thickness));

		return add_circle(position, direction, radius, color, segments, thickness);
	}

	BatchedLines& BatchedLines::add_box(const Vector3f& min, const Vector3f& max, const Color& color, float thickness)
	{
		add_line(Vertex({min.x, min.y, min.z}, color, thickness), Vertex({max.x, min.y, min.z}, color, thickness));
		add_line(Vertex({min.x, max.y, min.z}, color, thickness), Vertex({max.x, max.y, min.z}, color, thickness));
		add_line(Vertex({min.x, min.y, max.z}, color, thickness), Vertex({max.x, min.y, max.z}, color, thickness));
		add_line(Vertex({min.x, max.y, max.z}, color, thickness), Vertex({max.x, max.y, max.z}, color, thickness));

		add_line(Vertex({min.x, min.y, min.z}, color, thickness), Vertex({min.x, max.y, min.z}, color, thickness));
		add_line(Vertex({max.x, min.y, min.z}, color, thickness), Vertex({max.x, max.y, min.z}, color, thickness));
		add_line(Vertex({min.x, min.y, max.z}, color, thickness), Vertex({min.x, max.y, max.z}, color, thickness));
		add_line(Vertex({max.x, min.y, max.z}, color, thickness), Vertex({max.x, max.y, max.z}, color, thickness));

		add_line(Vertex({min.x, min.y, min.z}, color, thickness), Vertex({min.x, min.y, max.z}, color, thickness));
		add_line(Vertex({max.x, min.y, min.z}, color, thickness), Vertex({max.x, min.y, max.z}, color, thickness));
		add_line(Vertex({min.x, max.y, min.z}, color, thickness), Vertex({min.x, max.y, max.z}, color, thickness));
		add_line(Vertex({max.x, max.y, min.z}, color, thickness), Vertex({max.x, max.y, max.z}, color, thickness));
		return *this;
	}

	BatchedLines& BatchedLines::flush(RHIContext* ctx, Renderer* renderer)
	{
		if (m_first == nullptr)
			return *this;

		auto pool             = RHIBufferPool::global_instance();
		RHIBuffer* vtx_buffer = pool->request_buffer(s_line_vtx_per_node * sizeof(Vertex), s_vtx_buffer_flags);

#if TRINEX_DEBUG_BUILD
		ctx->push_debug_stage("Lines Rendering");
#endif

		auto pipeline = Pipelines::BatchedLines::instance();

		const Matrix4f& projview = renderer->scene_view().camera_view().projview;
		Vector2f size            = renderer->scene_view().view_size();

		ctx->bind_pipeline(pipeline->rhi_pipeline());
		ctx->update_scalar(&projview, pipeline->projview());
		ctx->update_scalar(&size, pipeline->viewport());

		ctx->bind_vertex_attribute(RHIVertexSemantic::Position, RHIVertexFormat::RGB32F, 0, 0);
		ctx->bind_vertex_attribute(RHIVertexSemantic::Color, RHIVertexFormat::RGBA8, 0, 12);
		ctx->bind_vertex_attribute(RHIVertexSemantic::UserData, RHIVertexFormat::R32F, 0, 16);

		ctx->bind_vertex_buffer(vtx_buffer, 0, sizeof(Vertex), 0);
		ctx->primitive_topology(RHIPrimitiveTopology::LineList);

		while (m_first)
		{
			ctx->barrier(vtx_buffer, RHIAccess::TransferDst);
			ctx->update_buffer(vtx_buffer, 0, m_first->vtx_count * sizeof(Vertex), reinterpret_cast<byte*>(m_first->vertices));
			ctx->barrier(vtx_buffer, RHIAccess::VertexBuffer);

			ctx->draw(m_first->vtx_count, 0);
			m_first = m_first->next;
		}

		m_last = nullptr;

		ctx->primitive_topology(RHIPrimitiveTopology::TriangleList);
#if TRINEX_DEBUG_BUILD
		ctx->pop_debug_stage();
#endif

		pool->return_buffer(vtx_buffer);
		return *this;
	}

	// TRIANGLES

	BatchedTriangles::BatchedTriangles()
	    : m_position_buffer(RHIBufferCreateFlags::Dynamic, 64, nullptr, true),
	      m_color_buffer(RHIBufferCreateFlags::Dynamic, 64, nullptr, true)
	{}

	BatchedTriangles& BatchedTriangles::clear()
	{
		m_vtx_count = 0;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::add_triangle(const Vector3f& point1, const Vector3f& point2, const Vector3f& point3,
	                                                 Color color1, Color color2, Color color3)
	{
		if (m_vtx_count + 3 > m_position_buffer.vertices())
		{
			m_position_buffer.grow();
			m_color_buffer.grow();
		}

		auto pos_dst = m_position_buffer.data() + m_vtx_count;
		auto col_dst = m_color_buffer.data() + m_vtx_count;

		new (pos_dst) Vector3f(point1);
		new (pos_dst + 1) Vector3f(point2);
		new (pos_dst + 2) Vector3f(point3);

		new (col_dst) Color(color1);
		new (col_dst + 1) Color(color2);
		new (col_dst + 2) Color(color3);

		m_vtx_count += 3;
		return *this;
	}

	BatchedTriangles& BatchedTriangles::render(RHIContext* ctx, Renderer* renderer)
	{
		if (m_vtx_count == 0)
			return *this;

		m_position_buffer.rhi_update(ctx, m_vtx_count * m_position_buffer.stride());
		m_color_buffer.rhi_update(ctx, m_vtx_count * m_color_buffer.stride());

		trinex_rhi_push_stage(ctx, "Triangles Rendering");

		ctx->bind_pipeline(Pipelines::BatchedTriangles::instance()->rhi_pipeline());
		ctx->bind_pipeline(Pipelines::BatchedTriangles::instance()->rhi_pipeline());
		ctx->bind_vertex_attribute(RHIVertexSemantic::Position, RHIVertexFormat::RGB32F, 0);
		ctx->bind_vertex_attribute(RHIVertexSemantic::Color, RHIVertexFormat::RGBA8, 1);

		ctx->bind_vertex_buffer(m_position_buffer.rhi_buffer(), 0, m_position_buffer.stride(), 0);
		ctx->bind_vertex_buffer(m_color_buffer.rhi_buffer(), 0, m_color_buffer.stride(), 1);

		ctx->draw(m_vtx_count, 0);

		trinex_rhi_pop_stage(ctx);
		return *this;
	}
}// namespace Engine
