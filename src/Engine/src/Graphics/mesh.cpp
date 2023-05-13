#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/mesh.hpp>
#include <numeric>

namespace Engine
{
    static size_t semantic_size_predicate(size_t last, const MeshSemanticEntry& entry)
    {
        return static_cast<size_t>(entry.count) + last;
    }

    StaticMeshSemanticInfo::StaticMeshSemanticInfo()
    {
        named_entries.vertex     = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.text_coord = MeshSemanticEntry::semantic_of<Vector2D>();
        named_entries.color      = MeshSemanticEntry::semantic_of<Vector4D>();
        named_entries.normal     = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.tangent    = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.binormal   = MeshSemanticEntry::semantic_of<Vector3D>();
    }

    DynamicMeshSemanticInfo::DynamicMeshSemanticInfo()
    {
        named_entries.vertex        = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.text_coord    = MeshSemanticEntry::semantic_of<Vector2D>();
        named_entries.color         = MeshSemanticEntry::semantic_of<Vector4D>();
        named_entries.normal        = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.tangent       = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.binormal      = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.blend_weight  = MeshSemanticEntry::semantic_of<Vector4D>();
        named_entries.blend_indices = MeshSemanticEntry::semantic_of<IntVector4D>();
    }


    template<size_t size>
    static size_t calculate_semantic_offset(const MeshSemanticEntry entries[size], VertexBufferSemantic semantic,
                                            byte index)
    {
        Index semantic_index = static_cast<Index>(semantic);
        if (semantic_index >= size || entries[semantic_index].count <= index)
            return Constants::index_none;

        return std::accumulate(entries, entries + semantic_index, 0, semantic_size_predicate) +
               entries[semantic_index].offset(index);
    }

    size_t StaticMeshSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshSemanticEntry)>(entries, semantic, index);
    }

    size_t DynamicMeshSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshSemanticEntry)>(entries, semantic, index);
    }

    const MeshSemanticInfo& StaticMesh::semantic_info() const
    {
        return info;
    }

    const MeshSemanticInfo& DynamicMesh::semantic_info() const
    {
        return info;
    }

    // Mesh serialization
    bool Mesh::MeshLOD::serialize(BufferWriter* writer) const
    {
        if (vertex_buffer == nullptr || index_buffer == nullptr)
        {
            error_log("MeshLOD: Cannot serialize resources!");
            return false;
        }

        if (!vertex_buffer->serialize(writer))
        {
            return false;
        }

        if (!index_buffer->serialize(writer))
        {
            return false;
        }

        return true;
    }

    bool Mesh::MeshLOD::deserialize(BufferReader* reader)
    {

        vertex_buffer = Object::new_instance<VertexBuffer>();

        if (!vertex_buffer->deserialize(reader))
        {
            Object::begin_destroy(vertex_buffer);
            vertex_buffer = nullptr;
            return false;
        }

        index_buffer = Object::new_instance<IndexBuffer>();

        if (!index_buffer->deserialize(reader))
        {
            Object::begin_destroy(index_buffer);
            index_buffer = nullptr;
            return false;
        }

        return true;
    }


    bool Mesh::serialize(BufferWriter* writer) const
    {
        if (!Object::serialize(writer))
        {
            return false;
        }

        if (!writer->write(lods))
        {
            error_log("Mesh: Failed to serialize lods!");
            return false;
        }

        return true;
    }

    bool Mesh::deserialize(BufferReader* reader)
    {
        if (!Object::deserialize(reader))
        {
            return false;
        }

        if (!reader->read(lods))
        {
            error_log("Mesh: Failed to deserialize lods!");
            return false;
        }

        return true;
    }

    Mesh::~Mesh()
    {
        for (auto& lod : lods)
        {
            if (lod.vertex_buffer)
            {
                Object::begin_destroy(lod.vertex_buffer);
                lod.vertex_buffer = nullptr;
            }

            if (lod.index_buffer)
            {
                Object::begin_destroy(lod.index_buffer);
                lod.index_buffer = nullptr;
            }
        }
    }

    bool StaticMesh::serialize(BufferWriter* writer) const
    {
        if (!Mesh::serialize(writer))
        {
            return false;
        }

        if (!writer->write(info))
        {
            error_log("StaticMesh: Failed to serialize mesh info");
            return false;
        }

        return true;
    }

    bool StaticMesh::deserialize(BufferReader* reader)
    {
        if (!Mesh::deserialize(reader))
        {
            return false;
        }

        if (!reader->read(info))
        {
            error_log("StaticMesh: Failed to deserialize mesh info");
            return false;
        }

        return true;
    }

    bool DynamicMesh::serialize(BufferWriter* writer) const
    {
        if (!Mesh::serialize(writer))
        {
            return false;
        }

        if (!writer->write(info))
        {
            error_log("DynamicMesh: Failed to serialize mesh info");
            return false;
        }

        return true;
    }

    bool DynamicMesh::deserialize(BufferReader* reader)
    {
        if (!Mesh::deserialize(reader))
        {
            return false;
        }

        if (!reader->read(info))
        {
            error_log("DynamicMesh: Failed to deserialize mesh info");
            return false;
        }

        return true;
    }


    register_class(Engine::Mesh, Engine::Object);
    register_class(Engine::StaticMesh, Engine::Mesh);
    register_class(Engine::DynamicMesh, Engine::Mesh);
}// namespace Engine
