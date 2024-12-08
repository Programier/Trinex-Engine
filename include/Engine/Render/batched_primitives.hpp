#pragma once
#include <Core/engine_types.hpp>
#include <Core/pointer.hpp>
#include <Graphics/pipeline_buffers.hpp>

namespace Engine
{
	class ENGINE_EXPORT BatchedPrimitive
	{
	protected:
		Pointer<PositionDynamicVertexBuffer> m_position_buffer = nullptr;
		Pointer<ColorDynamicVertexBuffer> m_color_buffer       = nullptr;
		size_t m_position_buffer_size                          = 0;
		size_t m_color_buffer_size                             = 0;

		bool begin_render();

	public:
		BatchedPrimitive();
		delete_copy_constructors(BatchedPrimitive);
		BatchedPrimitive& clear();
		~BatchedPrimitive();
	};

	class ENGINE_EXPORT BatchedLines final
	{
	public:
		struct ENGINE_EXPORT Vertex {
		private:
			ALIGNED(4) Vector3D m_position;
			ALIGNED(4) ByteColor4 m_color;
			ALIGNED(4) float m_thickness;

		public:
			FORCE_INLINE constexpr Vertex(const Vector3D& position = {0, 0, 0}, const ByteColor4& color = {255, 255, 255, 255},
			                              float thickness = 1.f)
			    : m_position(position), m_color(color), m_thickness(thickness)
			{}

			FORCE_INLINE const Vector3D& position() const
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

			FORCE_INLINE Vertex& position(const Vector3D& position)
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
		size_t m_vertex_count;

	public:
		BatchedLines();
		delete_copy_constructors(BatchedLines);

		BatchedLines& add_line(const Vertex& point1, const Vertex& point2);
		BatchedLines& render(const class SceneView& view);
		BatchedLines& clear();
	};

	class ENGINE_EXPORT BatchedTriangles : public BatchedPrimitive
	{
	public:
		BatchedTriangles& add_triangle(const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
		                               ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
		                               ByteColor color3 = {255, 255, 255, 255});
		BatchedTriangles& override_triangle(Index index, const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
		                                    ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
		                                    ByteColor color3 = {255, 255, 255, 255});
		BatchedTriangles& render(const class SceneView& view);
	};
}// namespace Engine
