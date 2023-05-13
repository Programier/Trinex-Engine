#pragma once
#include <Core/buffer_types.hpp>
#include <Core/constants.hpp>
#include <Core/pointer.hpp>
#include <Core/predef.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine
{
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
    };

    struct ENGINE_EXPORT StaticMeshSemanticInfo : public MeshSemanticInfo {
        union
        {
            struct {
                MeshSemanticEntry vertex;
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
    };

    struct ENGINE_EXPORT DynamicMeshSemanticInfo : public MeshSemanticInfo {
        union
        {
            struct {
                MeshSemanticEntry vertex;
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
    };


    class ENGINE_EXPORT Mesh : public Object
    {
    public:
        struct ENGINE_EXPORT MeshLOD : public SerializableObject {
            VertexBuffer* vertex_buffer = nullptr;
            IndexBuffer* index_buffer   = nullptr;

            bool serialize(BufferWriter* writer) const override;
            bool deserialize(BufferReader* reader) override;
        };

        Vector<MeshLOD> lods;
        virtual const MeshSemanticInfo& semantic_info() const = 0;

        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;

        ~Mesh();
    };


    class ENGINE_EXPORT StaticMesh : public Mesh
    {
    public:
        StaticMeshSemanticInfo info;

        const MeshSemanticInfo& semantic_info() const override;
        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;
    };

    class ENGINE_EXPORT DynamicMesh : public Mesh
    {
    public:
        DynamicMeshSemanticInfo info;

        const MeshSemanticInfo& semantic_info() const override;
        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;
    };
}// namespace Engine
