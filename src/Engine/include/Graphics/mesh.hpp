#pragma once
#include <Core/buffer_types.hpp>
#include <Core/pointer.hpp>
#include <Graphics/pipeline_buffers.hpp>


namespace Engine
{

    struct ENGINE_EXPORT MeshSemanticEntry {
    public:
        byte count;

    private:
        byte _M_variable_size;

    public:
        template<typename Type>
        static MeshSemanticEntry create()
        {
            MeshSemanticEntry entry;
            entry._M_variable_size = sizeof(Type);
            return entry;
        }
    };


    struct ENGINE_EXPORT MeshSemanticInfoBase {
        virtual size_t semantic_offset(VertexBufferSemantic semantic, byte index) = 0;
    };


    struct ENGINE_EXPORT StaticMeshSemanticInfo : public MeshSemanticInfoBase {
        union
        {
            struct {
                MeshSemanticEntry vertex;
                MeshSemanticEntry texcoord;
                MeshSemanticEntry color;
                MeshSemanticEntry normal;
                MeshSemanticEntry tangent;
                MeshSemanticEntry binormal;
                MeshSemanticEntry blend_weight;
                MeshSemanticEntry blend_indices;
            };

            MeshSemanticEntry entries[8];
        };

        virtual size_t semantic_offset(VertexBufferSemantic semantic, byte index) override;
    };


    class ENGINE_EXPORT MeshSemanticsInfo : public SerializableObject
    {
        //        union
        //        {
        //            struct {
        //                MeshSemanticEntry vertex(VertexBufferSemantic::Vertex);
        //                MeshSemanticEntry texcoord;
        //                MeshSemanticEntry color;
        //                MeshSemanticEntry normal;
        //                MeshSemanticEntry tangent;
        //                MeshSemanticEntry binormal;
        //                MeshSemanticEntry blend_weight;
        //                MeshSemanticEntry blend_indices;
        //            };

        //            MeshSemanticEntry entries[8];
        //        };

        bool serialize(BufferWriter* writer) override;
        bool deserialize(BufferReader* reader) override;
    };


    class ENGINE_EXPORT StaticMesh : public Object
    {
    public:
        struct ENGINE_EXPORT MeshLOD {
            Pointer<VertexBuffer> vertex_buffer;
            Vector<Pointer<IndexBuffer>> index_buffers;
        };

    public:
        MeshSemanticsInfo semantics_info;
        Vector<MeshLOD> lods;
    };
}// namespace Engine
