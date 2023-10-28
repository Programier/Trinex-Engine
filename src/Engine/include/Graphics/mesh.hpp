#pragma once
#include <Core/enums.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>

namespace Engine
{
    class MaterialInterface;
    class VertexBuffer;
    class IndexBuffer;

    class ENGINE_EXPORT StaticMesh : public Object
    {
        declare_class(StaticMesh, Object);

    public:
        struct ENGINE_EXPORT LOD {
            Vector<Pointer<class PositionVertexBuffer>> positions;
            Vector<Pointer<class TexCoordVertexBuffer>> tex_coords;
            Vector<Pointer<class ColorVertexBuffer>> color;
            Vector<Pointer<class NormalVertexBuffer>> normals;
            Vector<Pointer<class TangentVertexBuffer>> tangents;
            Vector<Pointer<class BinormalVertexBuffer>> binormal;

            Pointer<IndexBuffer> indices;
            Pointer<MaterialInterface> material;

            VertexBuffer* find_vertex_buffer(VertexBufferSemantic semantic, Index index = 0) const;
            const LOD& render() const;

        private:
            VertexBuffer* find_position_vertex_buffer(Index index) const;
            VertexBuffer* find_position_tex_coord_buffer(Index index) const;
            VertexBuffer* find_position_color_buffer(Index index) const;
            VertexBuffer* find_position_normal_buffer(Index index) const;
            VertexBuffer* find_position_tangent_buffer(Index index) const;
            VertexBuffer* find_position_binormal_buffer(Index index) const;
        };

        Vector<LOD> lods;
    };

    class ENGINE_EXPORT DynamicMesh : public Object
    {
        declare_class(DynamicMesh, Object);

    public:
        struct ENGINE_EXPORT RenderData {
        };

        struct ENGINE_EXPORT LOD {
            RenderData render_data;
            Pointer<MaterialInterface> material;
        };

        Vector<LOD> lods;
    };
}// namespace Engine
