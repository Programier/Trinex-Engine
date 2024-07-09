#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

namespace Engine
{
    class PositionDynamicVertexBuffer;
    class ColorDynamicVertexBuffer;

    class ENGINE_EXPORT BatchedPrimitive
    {
    protected:
        PositionDynamicVertexBuffer* m_position_buffer = nullptr;
        ColorDynamicVertexBuffer* m_color_buffer       = nullptr;
        size_t m_position_buffer_size                  = 0;
        size_t m_color_buffer_size                     = 0;

        bool begin_render();

    public:
        BatchedPrimitive();
        delete_copy_constructors(BatchedPrimitive);
        BatchedPrimitive& clear();
        ~BatchedPrimitive();
    };


    class ENGINE_EXPORT BatchedLines : public BatchedPrimitive
    {
    public:
        BatchedLines& add_line(const Vector3D& point1, const Vector3D& point2, ByteColor color1 = {255, 255, 255, 255},
                               ByteColor color2 = {255, 255, 255, 255});
        BatchedLines& override_line(Index index, const Vector3D& point1, const Vector3D& point2,
                                    ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255});
        BatchedLines& render(const class SceneView& view);
    };

    class ENGINE_EXPORT BatchedTriangles : public BatchedPrimitive
    {
    public:
        BatchedTriangles& add_triangle(const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
                                       ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
                                       ByteColor color3 = {255, 255, 255, 255});
        BatchedTriangles& override_line(Index index, const Vector3D& point1, const Vector3D& point2, const Vector3D& point3,
                                        ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255},
                                        ByteColor color3 = {255, 255, 255, 255});
        BatchedTriangles& render(const class SceneView& view);
    };
}// namespace Engine
