#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>

namespace Engine
{
    class PositionVertexBuffer;
    class ColorVertexBuffer;

    class ENGINE_EXPORT BatchedLines final
    {
    private:
        PositionVertexBuffer* m_position_buffer = nullptr;
        ColorVertexBuffer* m_color_buffer       = nullptr;
        size_t m_allocated_size                 = 0;

    public:
        BatchedLines();
        delete_copy_constructors(BatchedLines);

        BatchedLines& add_line(const Vector3D& point1, const Vector3D& point2, ByteColor color1 = {255, 255, 255, 255},
                               ByteColor color2 = {255, 255, 255, 255});
        BatchedLines& override_line(Index index, const Vector3D& point1, const Vector3D& point2,
                                    ByteColor color1 = {255, 255, 255, 255}, ByteColor color2 = {255, 255, 255, 255});
        BatchedLines& clear();
        BatchedLines& render(const class SceneView& view);
        ~BatchedLines();
    };
}// namespace Engine
