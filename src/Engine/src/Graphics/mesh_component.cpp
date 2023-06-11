#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/config.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Graphics/material.hpp>
#include <Graphics/mesh_component.hpp>
#include <numeric>
#include <strings.h>

namespace Engine
{
    static size_t semantic_size_predicate(size_t last, const MeshComponentSemanticEntry& entry)
    {
        return static_cast<size_t>(entry.count) * entry.type_size() + last;
    }

    StaticMeshComponentSemanticInfo::StaticMeshComponentSemanticInfo()
    {
        named_entries.position   = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.text_coord = MeshComponentSemanticEntry::semantic_of<Vector2D>();
        named_entries.color      = MeshComponentSemanticEntry::semantic_of<Vector4D>();
        named_entries.normal     = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.tangent    = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.binormal   = MeshComponentSemanticEntry::semantic_of<Vector3D>();
    }

    DynamicMeshComponentSemanticInfo::DynamicMeshComponentSemanticInfo()
    {
        named_entries.position      = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.text_coord    = MeshComponentSemanticEntry::semantic_of<Vector2D>();
        named_entries.color         = MeshComponentSemanticEntry::semantic_of<Vector4D>();
        named_entries.normal        = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.tangent       = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.binormal      = MeshComponentSemanticEntry::semantic_of<Vector3D>();
        named_entries.blend_weight  = MeshComponentSemanticEntry::semantic_of<Vector4D>();
        named_entries.blend_indices = MeshComponentSemanticEntry::semantic_of<IntVector4D>();
    }


    template<size_t size>
    static size_t calculate_semantic_offset(const MeshComponentSemanticEntry entries[size],
                                            VertexBufferSemantic semantic, byte index)
    {
        Index semantic_index = static_cast<Index>(semantic);
        if (semantic_index >= size || entries[semantic_index].count <= index)
            return Constants::index_none;

        return std::accumulate(entries, entries + semantic_index, 0, semantic_size_predicate) +
               entries[semantic_index].offset(index);
    }

    template<size_t size>
    static size_t calculate_vertex_size(const MeshComponentSemanticEntry entries[size])
    {
        return std::accumulate(entries, entries + size, 0, semantic_size_predicate);
    }

    template<size_t size>
    static MeshComponentSemanticEntry& semantic_entry_of(MeshComponentSemanticEntry entries[size],
                                                         VertexBufferSemantic semantic)
    {
        Index index = static_cast<Index>(semantic);
        if (index >= size)
        {
            throw EngineException("MeshComponentSemanticEntry: Cannot find semantic");
        }
        return entries[index];
    }


    size_t StaticMeshComponentSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshComponentSemanticEntry)>(entries, semantic,
                                                                                               index);
    }

    size_t StaticMeshComponentSemanticInfo::vertex_size() const
    {
        return calculate_vertex_size<6>(entries);
    }

    MeshComponentSemanticEntry& StaticMeshComponentSemanticInfo::entry_of(VertexBufferSemantic semantic)
    {
        return semantic_entry_of<6>(entries, semantic);
    }

    size_t DynamicMeshComponentSemanticInfo::semantic_offset(VertexBufferSemantic semantic, byte index) const
    {
        return calculate_semantic_offset<sizeof(entries) / sizeof(MeshComponentSemanticEntry)>(entries, semantic,
                                                                                               index);
    }

    size_t DynamicMeshComponentSemanticInfo::vertex_size() const
    {
        return calculate_vertex_size<8>(entries);
    }

    MeshComponentSemanticEntry& DynamicMeshComponentSemanticInfo::entry_of(VertexBufferSemantic semantic)
    {
        return semantic_entry_of<8>(entries, semantic);
    }


    const StaticMeshComponentSemanticInfo& StaticMeshComponent::semantic_info() const
    {
        return info;
    }

    const MeshComponentSemanticInfo& DynamicMeshComponent::semantic_info() const
    {
        return info;
    }


    bool MeshComponent::MeshComponentLOD::archive_process(Archive* archive)
    {
        if (!vertex_buffer.archive_process(archive) || !index_buffer.archive_process(archive))
        {
            return false;
        }

        if (!material_reference.archive_process(archive))
            return false;

        return static_cast<bool>(*archive);
    }


    void MeshComponent::create_appliers(Archive* archive)
    {
        if (archive->is_reading() && engine_instance->api() != EngineAPI::NoAPI && engine_config.load_shaders_to_gpu)
        {
            for (auto& lod : lods)
            {
                lod._M_material_applier = lod.material_reference.instance()->create_material_applier(this);
            }
        }
    }

    bool MeshComponent::archive_process(Archive* archive)
    {
        if (!Object::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & lods))
        {
            error_log("MeshComponent: Failed to process lods!");
            return false;
        }

        return static_cast<bool>(*archive);
    }

    MaterialApplier* MeshComponent::material_applier(Index lod) const
    {
        if (lod >= lods.size())
            return nullptr;
        return lods[lod]._M_material_applier;
    }

    MeshComponent::~MeshComponent()
    {}

    bool StaticMeshComponent::archive_process(Archive* archive)
    {
        if (!MeshComponent::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & info.entries))
        {
            error_log("StaticMeshComponent: Failed to process MeshComponent info");
            return false;
        }

        create_appliers(archive);
        return static_cast<bool>(*archive);
    }

    bool DynamicMeshComponent::archive_process(Archive* archive)
    {
        if (!MeshComponent::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & info.entries))
        {
            error_log("StaticMeshComponent: Failed to process MeshComponent info");
            return false;
        }

        create_appliers(archive);
        return static_cast<bool>(*archive);
    }


    register_class(Engine::MeshComponent);
    register_class(Engine::StaticMeshComponent);
    register_class(Engine::DynamicMeshComponent);
}// namespace Engine
