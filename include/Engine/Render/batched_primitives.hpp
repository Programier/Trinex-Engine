#pragma once
#include <Core/engine_types.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class RendererContext;

	class ENGINE_EXPORT BatchedLines final
	{
	public:
		struct ENGINE_EXPORT Vertex {
		private:
			ALIGNED(4) Vector3f m_position;
			ALIGNED(4) Color m_color;
			ALIGNED(4) float m_thickness;

		public:
			FORCE_INLINE constexpr Vertex(const Vector3f& position = {0, 0, 0}, const Color& color = {255, 255, 255, 255},
			                              float thickness = 1.f)
			    : m_position(position), m_color(color), m_thickness(thickness)
			{}

			FORCE_INLINE const Vector3f& position() const { return m_position; }
			FORCE_INLINE const Color& color() const { return m_color; }
			FORCE_INLINE float thickness() const { return m_thickness; }
			FORCE_INLINE Vertex& position(const Vector3f& position)
			{
				m_position = position;
				return *this;
			}

			FORCE_INLINE Vertex& color(const Color& color)
			{
				m_color = color;
				return *this;
			}

			FORCE_INLINE Vertex& thickness(float thickness)
			{
				m_thickness = thickness;
				return *this;
			}
		};

	private:
		VertexBuffer<Vertex> m_vtx_buffer;
		size_t m_vtx_count = 0;

	public:
		BatchedLines();
		delete_copy_constructors(BatchedLines);

		BatchedLines& add_line(const Vertex& point1, const Vertex& point2);
		BatchedLines& render(const RendererContext& ctx);
		BatchedLines& clear();
	};

	class ENGINE_EXPORT BatchedTriangles final
	{
		PositionVertexBuffer m_position_buffer;
		ColorVertexBuffer m_color_buffer;
		size_t m_vtx_count = 0;

	public:
		BatchedTriangles();
		delete_copy_constructors(BatchedTriangles);
		BatchedTriangles& clear();

		BatchedTriangles& add_triangle(const Vector3f& point1, const Vector3f& point2, const Vector3f& point3,
		                               Color color1 = {255, 255, 255, 255}, Color color2 = {255, 255, 255, 255},
		                               Color color3 = {255, 255, 255, 255});
		BatchedTriangles& render(const RendererContext& ctx);
	};
}// namespace Engine
