#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class Renderer;
	class RHIContext;

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

			FORCE_INLINE constexpr Vertex(const Vector2f& position, const Color& color = {255, 255, 255, 255},
			                              float thickness = 1.f)
			    : m_position(position, 0.f), m_color(color), m_thickness(thickness)
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
		struct Node {
			Node* next;
			Vertex* vertices;
			size_t vtx_count;

			Node();
		};

		Node* m_first = nullptr;
		Node* m_last  = nullptr;

		Node* find_node();

	public:
		BatchedLines();
		delete_copy_constructors(BatchedLines);

		BatchedLines& add_line(const Vertex& point1, const Vertex& point2);
		BatchedLines& add_circle(const Vector3f& position, const Vector3f& normal, float radius,
		                         const Color& color = {255, 255, 255, 255}, uint_t segments = 0, float thickness = 1.f);
		BatchedLines& add_sphere(const Vector3f& position, float radius, const Color& color = {255, 255, 255, 255},
		                         uint_t segments = 0, float thickness = 1.f);
		BatchedLines& add_arrow(const Vector3f& position, const Vector3f& direction, const Color& color = {255, 255, 255, 255},
		                        float thickness = 1.f);
		BatchedLines& add_cone(const Vector3f& position, const Vector3f& direction, float radius,
		                       const Color& color = {255, 255, 255, 255}, uint_t segments = 0, float thickness = 1.f);
		BatchedLines& add_box(const Vector3f& min, const Vector3f& max, const Color& color = {255, 255, 255, 255},
		                      float thickness = 1.f);

		BatchedLines& flush(RHIContext* ctx, Renderer* renderer);

		inline bool is_empty() const { return m_first == nullptr; }
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
		BatchedTriangles& render(RHIContext* ctx, Renderer* renderer);
	};
}// namespace Engine
