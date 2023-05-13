#include <Core/buffer_manager.hpp>
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
        names_entries.vertex     = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.text_coord = MeshSemanticEntry::semantic_of<Vector2D>();
        names_entries.color      = MeshSemanticEntry::semantic_of<Vector4D>();
        names_entries.normal     = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.tangent    = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.binormal   = MeshSemanticEntry::semantic_of<Vector3D>();
    }

    DynamicMeshSemanticInfo::DynamicMeshSemanticInfo()
    {
        names_entries.vertex        = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.text_coord    = MeshSemanticEntry::semantic_of<Vector2D>();
        names_entries.color         = MeshSemanticEntry::semantic_of<Vector4D>();
        names_entries.normal        = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.tangent       = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.binormal      = MeshSemanticEntry::semantic_of<Vector3D>();
        names_entries.blend_weight  = MeshSemanticEntry::semantic_of<Vector4D>();
        names_entries.blend_indices = MeshSemanticEntry::semantic_of<IntVector4D>();
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
}// namespace Engine
