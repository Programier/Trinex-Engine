#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/mesh.hpp>
#include <numeric>
#include <strings.h>

namespace Engine
{
    static size_t semantic_size_predicate(size_t last, const MeshSemanticEntry& entry)
    {
        return static_cast<size_t>(entry.count) * entry.type_size() + last;
    }

    StaticMeshSemanticInfo::StaticMeshSemanticInfo()
    {
        named_entries.position   = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.text_coord = MeshSemanticEntry::semantic_of<Vector2D>();
        named_entries.color      = MeshSemanticEntry::semantic_of<Vector4D>();
        named_entries.normal     = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.tangent    = MeshSemanticEntry::semantic_of<Vector3D>();
        named_entries.binormal   = MeshSemanticEntry::semantic_of<Vector3D>();
    }

    DynamicMeshSemanticInfo::DynamicMeshSemanticInfo()
    {
        named_entries.position      = MeshSemanticEntry::semantic_of<Vector3D>();
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

    template<size_t size>
    static size_t calculate_vertex_size(const MeshSemanticEntry entries[size])
    {
        return std::accumulate(entries, entries + size, 0, semantic_size_predicate);
    }


    size_t StaticMeshSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshSemanticEntry)>(entries, semantic, index);
    }

    size_t StaticMeshSemanticInfo::vertex_size() const
    {
        return calculate_vertex_size<6>(entries);
    }

    size_t DynamicMeshSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshSemanticEntry)>(entries, semantic, index);
    }

    size_t DynamicMeshSemanticInfo::vertex_size() const
    {
        return calculate_vertex_size<8>(entries);
    }


    const StaticMeshSemanticInfo& StaticMesh::semantic_info() const
    {
        return info;
    }

    const MeshSemanticInfo& DynamicMesh::semantic_info() const
    {
        return info;
    }


    bool Mesh::MeshLOD::archive_process(Archive* archive)
    {
        if (archive->is_reading())
        {
            if (vertex_buffer == nullptr)
                vertex_buffer = Object::new_instance<VertexBuffer>();
            if (index_buffer == nullptr)
                index_buffer = Object::new_instance<IndexBuffer>();
        }

        if (!vertex_buffer || !index_buffer)
        {
            error_log("MeshLOD: Cannot process resources!");
        }

        if (!vertex_buffer->archive_process(archive) || !index_buffer->archive_process(archive))
        {
            Object::begin_destroy(vertex_buffer);
            Object::begin_destroy(index_buffer);
            return false;
        }

        return static_cast<bool>(*archive);
    }

    bool Mesh::archive_process(Archive* archive)
    {
        if (!Object::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & lods))
        {
            error_log("Mesh: Failed to process lods!");
            return false;
        }

        return static_cast<bool>(*archive);
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


    bool StaticMesh::archive_process(Archive* archive)
    {
        if (!Mesh::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & info.entries))
        {
            error_log("StaticMesh: Failed to process mesh info");
            return false;
        }
        return static_cast<bool>(*archive);
    }

    bool DynamicMesh::archive_process(Archive* archive)
    {
        if (!Mesh::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & info.entries))
        {
            error_log("StaticMesh: Failed to process mesh info");
            return false;
        }
        return static_cast<bool>(*archive);
    }


    register_class(Engine::Mesh, Engine::Object);
    register_class(Engine::StaticMesh, Engine::Mesh);
    register_class(Engine::DynamicMesh, Engine::Mesh);
}// namespace Engine
