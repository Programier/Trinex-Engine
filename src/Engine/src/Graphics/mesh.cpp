#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
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


    VertexBuffer* StaticMesh::find_position_buffer(Index index) const
    {
        return positions.size() <= index ? nullptr : positions[index].ptr();
    }

    VertexBuffer* StaticMesh::find_tex_coord_buffer(Index index) const
    {
        return tex_coords.size() <= index ? nullptr : tex_coords[index].ptr();
    }

    VertexBuffer* StaticMesh::find_color_buffer(Index index) const
    {
        return colors.size() <= index ? nullptr : colors[index].ptr();
    }

    VertexBuffer* StaticMesh::find_normal_buffer(Index index) const
    {
        return normals.size() <= index ? nullptr : normals[index].ptr();
    }

    VertexBuffer* StaticMesh::find_tangent_buffer(Index index) const
    {
        return tangents.size() <= index ? nullptr : tangents[index].ptr();
    }

    VertexBuffer* StaticMesh::find_binormal_buffer(Index index) const
    {
        return binormals.size() <= index ? nullptr : binormals[index].ptr();
    }


    StaticMesh& StaticMesh::init_resources()
    {
        for (auto& position : positions)
        {
            if (position)
                position->init_resource();
        }

        for (auto& coord : tex_coords)
        {
            if (coord)
                coord->init_resource();
        }

        for (auto& color : colors)
        {
            if (color)
                color->init_resource();
        }

        for (auto& normal : normals)
        {
            if (normal)
                normal->init_resource();
        }

        for (auto& tangent : tangents)
        {
            if (tangent)
                tangent->init_resource();
        }

        for (auto& binormal : binormals)
        {
            if (binormal)
                binormal->init_resource();
        }

        if (indices)
        {
            indices->init_resource();
        }

        return *this;
    }

    StaticMesh& StaticMesh::apply_changes()
    {
        return init_resources();
    }

    VertexBuffer* StaticMesh::find_vertex_buffer(VertexBufferSemantic semantic, Index index) const
    {
        Index semantic_index = static_cast<Index>(semantic);
        if (semantic_index > static_cast<Index>(VertexBufferSemantic::Binormal))
        {
            return nullptr;
        }

        static VertexBuffer* (StaticMesh::*find_buffer_private[])(Index) const = {
                &StaticMesh::find_position_buffer, &StaticMesh::find_tex_coord_buffer, &StaticMesh::find_color_buffer,
                &StaticMesh::find_normal_buffer,   &StaticMesh::find_tangent_buffer,   &StaticMesh::find_binormal_buffer,
        };

        return ((*this).*(find_buffer_private[semantic_index]))(index);
    }
}// namespace Engine
