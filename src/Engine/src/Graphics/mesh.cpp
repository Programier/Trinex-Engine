#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
    implement_class(StaticMesh, "Engine", 0);
    implement_class(DynamicMesh, "Engine", 0);
    implement_default_initialize_class(StaticMesh);
    implement_default_initialize_class(DynamicMesh);


    VertexBuffer* StaticMesh::LOD::find_position_vertex_buffer(Index index) const
    {
        return positions.size() <= index ? nullptr : positions[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_position_tex_coord_buffer(Index index) const
    {
        return tex_coords.size() <= index ? nullptr : tex_coords[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_position_color_buffer(Index index) const
    {
        return color.size() <= index ? nullptr : color[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_position_normal_buffer(Index index) const
    {
        return normals.size() <= index ? nullptr : normals[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_position_tangent_buffer(Index index) const
    {
        return tangents.size() <= index ? nullptr : tangents[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_position_binormal_buffer(Index index) const
    {
        return binormal.size() <= index ? nullptr : binormal[index].ptr();
    }


    VertexBuffer* StaticMesh::LOD::find_vertex_buffer(VertexBufferSemantic semantic, Index index) const
    {
        Index semantic_index = static_cast<Index>(semantic);
        if (semantic_index > static_cast<Index>(VertexBufferSemantic::Binormal))
        {
            return nullptr;
        }

        static VertexBuffer* (LOD::*find_buffer_private[])(Index) const = {
                &LOD::find_position_vertex_buffer, &LOD::find_position_tex_coord_buffer, &LOD::find_position_color_buffer,
                &LOD::find_position_normal_buffer, &LOD::find_position_tangent_buffer,   &LOD::find_position_binormal_buffer,
        };

        return ((*this).*(find_buffer_private[semantic_index]))(index);
    }
}// namespace Engine
