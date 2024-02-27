#include <Core/archive.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/property.hpp>
#include <Engine/Render/scene_renderer.hpp>
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


    StaticMesh::StaticMesh()
    {
        material = Object::find_object_checked<MaterialInterface>("DefaultPackage::DefaultMaterial");
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

    StaticMesh& StaticMesh::postload()
    {
        return init_resources();
    }


    template<typename Type>
    static void serialize_buffer(Archive& ar, Pointer<Type>& buffer)
    {
        bool is_valid = buffer;
        ar & is_valid;

        if (is_valid)
        {
            if (ar.is_reading())
            {
                buffer = Object::new_instance<Type>();
            }

            buffer.ptr()->archive_process(ar);
        }
    }

    template<typename Type>
    static void serialize_buffers(Archive& ar, Vector<Pointer<Type>>& buffers)
    {
        size_t size = buffers.size();
        ar & size;

        if (size > 0)
        {
            if (ar.is_reading())
            {
                buffers.resize(size);
            }

            for (auto& buffer : buffers)
            {
                serialize_buffer(ar, buffer);
            }
        }
    }

    ENGINE_EXPORT bool operator&(Archive& ar, StaticMesh::LOD& lod)
    {
        serialize_buffers(ar, lod.positions);
        serialize_buffers(ar, lod.tex_coords);
        serialize_buffers(ar, lod.colors);
        serialize_buffers(ar, lod.normals);
        serialize_buffers(ar, lod.tangents);
        serialize_buffers(ar, lod.binormals);
        serialize_buffer(ar, lod.indices);
        return ar;
    }

    bool StaticMesh::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;

        ar & bounds;
        ar & lods;

        return ar;
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
