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

    struct MeshComponentSemanticEntry {
        byte count = 0;

    private:
        byte _M_size = 0;

    public:
        template<typename Type>
        static MeshComponentSemanticEntry semantic_of()
        {
            MeshComponentSemanticEntry result;
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


    class ENGINE_EXPORT MeshComponentSemanticInfo
    {
    public:
        virtual size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const = 0;
        virtual size_t vertex_size() const                                                  = 0;
        virtual MeshComponentSemanticEntry& entry_of(VertexBufferSemantic)                           = 0;
        ~MeshComponentSemanticInfo()
        {}
    };

    struct ENGINE_EXPORT StaticMeshComponentSemanticInfo : public MeshComponentSemanticInfo {
        union
        {
            struct {
                MeshComponentSemanticEntry position;
                MeshComponentSemanticEntry text_coord;
                MeshComponentSemanticEntry color;
                MeshComponentSemanticEntry normal;
                MeshComponentSemanticEntry tangent;
                MeshComponentSemanticEntry binormal;
            } named_entries;

            MeshComponentSemanticEntry entries[6];
        };

        StaticMeshComponentSemanticInfo();
        size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const override;
        size_t vertex_size() const override;
        MeshComponentSemanticEntry& entry_of(VertexBufferSemantic) override;
    };

    struct ENGINE_EXPORT DynamicMeshComponentSemanticInfo : public MeshComponentSemanticInfo {
        union
        {
            struct {
                MeshComponentSemanticEntry position;
                MeshComponentSemanticEntry text_coord;
                MeshComponentSemanticEntry color;
                MeshComponentSemanticEntry normal;
                MeshComponentSemanticEntry tangent;
                MeshComponentSemanticEntry binormal;
                MeshComponentSemanticEntry blend_weight;
                MeshComponentSemanticEntry blend_indices;
            } named_entries;

            MeshComponentSemanticEntry entries[8];
        };

        DynamicMeshComponentSemanticInfo();
        size_t semantic_offset(VertexBufferSemantic semantic, byte index = 0) const override;
        size_t vertex_size() const override;
        MeshComponentSemanticEntry& entry_of(VertexBufferSemantic) override;
    };


    class ENGINE_EXPORT MeshComponent : public Object
    {
    public:
        using Super = Object;

        struct ENGINE_EXPORT MeshComponentLOD : public SerializableObject {
            ObjectReference<Material> material_reference;
            VertexBuffer vertex_buffer;
            IndexBuffer index_buffer;

            bool archive_process(Archive* archive) override;

            friend class MeshComponent;

        private:
            MaterialApplier* _M_material_applier = nullptr;
        };

    protected:
        void create_appliers(Archive*);

    public:
        Vector<MeshComponentLOD> lods;
        virtual const MeshComponentSemanticInfo& semantic_info() const = 0;
        bool archive_process(Archive* archive) override;
        MaterialApplier* material_applier(Index lod) const;

        ~MeshComponent();
    };


    class ENGINE_EXPORT StaticMeshComponent : public MeshComponent
    {
    public:
        using Super = MeshComponent;

        StaticMeshComponentSemanticInfo info;

        const StaticMeshComponentSemanticInfo& semantic_info() const override;
        bool archive_process(Archive* archive) override;
    };

    class ENGINE_EXPORT DynamicMeshComponent : public MeshComponent
    {
    public:
        using Super = MeshComponent;

        DynamicMeshComponentSemanticInfo info;

        const MeshComponentSemanticInfo& semantic_info() const override;
        bool archive_process(Archive* archive) override;
    };
}// namespace Engine
