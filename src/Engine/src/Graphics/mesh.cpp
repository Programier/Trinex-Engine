#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Engine/scene_renderer.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{
    implement_class(StaticMesh, Engine, Class::IsAsset);
    implement_class(DynamicMesh, Engine, 0);

    implement_initialize_class(StaticMesh)
    {
        Class* self = StaticMesh::static_class_instance();
        self->add_property(new ObjectReferenceProperty("Material", "Material which used for rendering this primitive",
                                                       &StaticMesh::material));
    }

    implement_default_initialize_class(DynamicMesh);


    VertexBuffer* StaticMesh::LOD::find_position_buffer(Index index) const
    {
        return positions.size() <= index ? nullptr : positions[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_tex_coord_buffer(Index index) const
    {
        return tex_coords.size() <= index ? nullptr : tex_coords[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_color_buffer(Index index) const
    {
        return colors.size() <= index ? nullptr : colors[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_normal_buffer(Index index) const
    {
        return normals.size() <= index ? nullptr : normals[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_tangent_buffer(Index index) const
    {
        return tangents.size() <= index ? nullptr : tangents[index].ptr();
    }

    VertexBuffer* StaticMesh::LOD::find_binormal_buffer(Index index) const
    {
        return binormals.size() <= index ? nullptr : binormals[index].ptr();
    }


    StaticMesh& StaticMesh::init_resources()
    {
        for (auto& lod : lods)
        {
            for (auto& position : lod.positions)
            {
                if (position)
                    position->init_resource();
            }

            for (auto& coord : lod.tex_coords)
            {
                if (coord)
                    coord->init_resource();
            }

            for (auto& color : lod.colors)
            {
                if (color)
                    color->init_resource();
            }

            for (auto& normal : lod.normals)
            {
                if (normal)
                    normal->init_resource();
            }

            for (auto& tangent : lod.tangents)
            {
                if (tangent)
                    tangent->init_resource();
            }

            for (auto& binormal : lod.binormals)
            {
                if (binormal)
                    binormal->init_resource();
            }

            if (lod.indices)
            {
                lod.indices->init_resource();
            }
        }

        return *this;
    }

    StaticMesh& StaticMesh::apply_changes()
    {
        return init_resources();
    }

    VertexBuffer* StaticMesh::LOD::find_vertex_buffer(VertexBufferSemantic semantic, Index index) const
    {
        Index semantic_index = static_cast<Index>(semantic);
        if (semantic_index > static_cast<Index>(VertexBufferSemantic::Binormal))
        {
            return nullptr;
        }

        static VertexBuffer* (StaticMesh::LOD::*find_buffer_private[])(Index) const = {
                &StaticMesh::LOD::find_position_buffer, &StaticMesh::LOD::find_tex_coord_buffer,
                &StaticMesh::LOD::find_color_buffer,    &StaticMesh::LOD::find_normal_buffer,
                &StaticMesh::LOD::find_tangent_buffer,  &StaticMesh::LOD::find_binormal_buffer,
        };

        return ((*this).*(find_buffer_private[semantic_index]))(index);
    }
}// namespace Engine
