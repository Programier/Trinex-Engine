#pragma once
#include <Core/buffer_types.hpp>
#include <Core/constants.hpp>
#include <Core/object_ref.hpp>
#include <Core/pointer.hpp>
#include <Core/predef.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine
{
    class Material;
    class MaterialApplier;

    struct MeshSemanticEntry {
        byte count = 0;

    private:
        byte _M_size = 0;

    public:
        template<typename Type>
        static MeshSemanticEntry semantic_of()
        {
            MeshSemanticEntry result;
            result.count   = 0;
            result._M_size = static_cast<byte>(sizeof(Type));
            return result;
        }

        FORCE_INLINE size_t type_size() const
        {
            return static_cast<size_t>(_M_size);
        }

        FORCE_INLINE size_t semantic_size() const
        {
            return type_size() * static_cast<size_t>(count);
        }

        FORCE_INLINE size_t offset(byte index) const
        {
            if (count <= index)
                return Constants::index_none;

            return _M_size * static_cast<size_t>(index);
        }
    };


    class ENGINE_EXPORT MeshSemanticInfo
    {
    public:
        virtual size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const = 0;
        virtual size_t vertex_size() const                                                  = 0;
        virtual MeshSemanticEntry& entry_of(VertexBufferSemantic)                           = 0;
        ~MeshSemanticInfo()
        {}
    };

    struct ENGINE_EXPORT StaticMeshSemanticInfo : public MeshSemanticInfo {
        union
        {
            struct {
                MeshSemanticEntry position;
                MeshSemanticEntry text_coord;
                MeshSemanticEntry color;
                MeshSemanticEntry normal;
                MeshSemanticEntry tangent;
                MeshSemanticEntry binormal;
            } named_entries;

            MeshSemanticEntry entries[6];
        };

        StaticMeshSemanticInfo();
        size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const override;
        size_t vertex_size() const override;
        MeshSemanticEntry& entry_of(VertexBufferSemantic) override;
    };

    struct ENGINE_EXPORT DynamicMeshSemanticInfo : public MeshSemanticInfo {
        union
        {
            struct {
                MeshSemanticEntry position;
                MeshSemanticEntry text_coord;
                MeshSemanticEntry color;
                MeshSemanticEntry normal;
                MeshSemanticEntry tangent;
                MeshSemanticEntry binormal;
                MeshSemanticEntry blend_weight;
                MeshSemanticEntry blend_indices;
            } named_entries;

            MeshSemanticEntry entries[8];
        };

        DynamicMeshSemanticInfo();
        size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const override;
        size_t vertex_size() const override;
        MeshSemanticEntry& entry_of(VertexBufferSemantic) override;
    };


    class ENGINE_EXPORT Mesh : public Object
    {
    public:
        struct ENGINE_EXPORT MeshLOD : public SerializableObject {
            ObjectReference<Material> material_reference;
            VertexBuffer vertex_buffer;
            IndexBuffer index_buffer;

            bool archive_process(Archive* archive) override;

            friend class Mesh;

        private:
            MaterialApplier* _M_material_applier = nullptr;
        };

    protected:
        void create_appliers(Archive*);

    public:
        Vector<MeshLOD> lods;
        virtual const MeshSemanticInfo& semantic_info() const = 0;
        bool archive_process(Archive* archive) override;
        MaterialApplier* material_applier(Index lod) const;

        ~Mesh();
    };


    class ENGINE_EXPORT StaticMesh : public Mesh
    {
    public:
        StaticMeshSemanticInfo info;

        const StaticMeshSemanticInfo& semantic_info() const override;
        bool archive_process(Archive* archive) override;
    };

    class ENGINE_EXPORT DynamicMesh : public Mesh
    {
    public:
        DynamicMeshSemanticInfo info;

        const MeshSemanticInfo& semantic_info() const override;
        bool archive_process(Archive* archive) override;
    };
}// namespace Engine
