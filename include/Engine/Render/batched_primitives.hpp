#pragma once
#include <Core/engine_types.hpp>
#include <Core/pointer.hpp>
#include <Graphics/gpu_buffers.hpp>

namespace Engine
{
	class ENGINE_EXPORT BatchedLines final
	{
	public:
		struct ENGINE_EXPORT Vertex {
		private:
			ALIGNED(4) Vector3f m_position;
			ALIGNED(4) ByteColor4 m_color;
			ALIGNED(4) float m_thickness;

		public:
			FORCE_INLINE constexpr Vertex(const Vector3f& position = {0, 0, 0}, const ByteColor4& color = {255, 255, 255, 255},
			                              float thickness = 1.f)
			    : m_position(position), m_color(color), m_thickness(thickness)
			{}

			FORCE_INLINE const Vector3f& position() const
			{
				return m_position;
			}

			FORCE_INLINE const ByteColor4& color() const
			{
				return m_color;
			}

			FORCE_INLINE float thickness() const
			{
				return m_thickness;
			}

			FORCE_INLINE Vertex& position(const Vector3f& position)
			{
				m_position = position;
				return *this;
			}

			FORCE_INLINE Vertex& color(const ByteColor4& color)
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

		using LinesVertexBuffer = TypedDynamicVertexBuffer<Vertex>;

	private:
		Pointer<LinesVertexBuffer> m_lines;

	public:
		BatchedLines();
		delete_copy_constructors(BatchedLines);

		BatchedLines& add_line(const Vertex& point1, const Vertex& point2);
		BatchedLines& render(class RenderPass* pass);
		BatchedLines& clear();
	};

	class ENGINE_EXPORT BatchedTriangles final
	{
		Pointer<PositionDynamicVertexBuffer> m_position_buffer = nullptr;
		Pointer<ColorDynamicVertexBuffer> m_color_buffer       = nullptr;

	public:
		BatchedTriangles();
		delete_copy_constructors(BatchedTriangles);
		BatchedTriangles& clear();

		BatchedTriangles& add_triangle(const Vector3f& point1, const Vector3f& point2, const Vector3f& point3,
		                               ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
		                               ByteColor color3 = {255, 255, 255, 255});
		BatchedTriangles& override_triangle(Index index, const Vector3f& point1, const Vector3f& point2, const Vector3f& point3,
		                                    ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
		                                    ByteColor color3 = {255, 255, 255, 255});
		BatchedTriangles& render(class RenderPass* pass);
	};
}// namespace Engine
