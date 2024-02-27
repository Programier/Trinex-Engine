#pragma once
#include <Core/enums.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
    class MaterialInterface;
    class VertexBuffer;
    class IndexBuffer;

    class ENGINE_EXPORT StaticMesh : public Object
    {
        declare_class(StaticMesh, Object);

    private:
    public:
        MaterialInterface* material = nullptr;

        struct ENGINE_EXPORT LOD {
            Vector<Pointer<class PositionVertexBuffer>> positions;
            Vector<Pointer<class TexCoordVertexBuffer>> tex_coords;
            Vector<Pointer<class ColorVertexBuffer>> colors;
            Vector<Pointer<class NormalVertexBuffer>> normals;
            Vector<Pointer<class TangentVertexBuffer>> tangents;
            Vector<Pointer<class BinormalVertexBuffer>> binormals;
            Pointer<IndexBuffer> indices;


        private:
            VertexBuffer* find_position_buffer(Index index) const;
            VertexBuffer* find_tex_coord_buffer(Index index) const;
            VertexBuffer* find_color_buffer(Index index) const;
            VertexBuffer* find_normal_buffer(Index index) const;
            VertexBuffer* find_tangent_buffer(Index index) const;
            VertexBuffer* find_binormal_buffer(Index index) const;

        public:
            VertexBuffer* find_vertex_buffer(VertexBufferSemantic semantic, Index index = 0) const;
        };

        AABB_3Df bounds;
        Vector<LOD> lods;

        StaticMesh();
        StaticMesh& init_resources();
        StaticMesh& apply_changes() override;
        bool archive_process(Archive& ar) override;
        StaticMesh& postload() override;
    };

    ENGINE_EXPORT bool operator& (Archive& ar, StaticMesh::LOD& lod);

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
