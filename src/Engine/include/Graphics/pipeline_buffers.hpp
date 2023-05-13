#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_types.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/resource.hpp>

namespace Engine
{
    class ENGINE_EXPORT PipelineBuffer : public Resource<Buffer, ApiObjectNoBase>, SerializableObject
    {
    protected:
        size_t _M_size = 0;

    public:
        inline size_t size() const
        {
            return _M_size;
        }

        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;
    };

    class ENGINE_EXPORT VertexBuffer : public PipelineBuffer
    {
    public:
        VertexBuffer();
        delete_copy_constructors(VertexBuffer);
        virtual VertexBuffer& create(const byte* data, size_t size);
        VertexBuffer& update(size_t offset, const byte* data, size_t size);
        VertexBuffer& bind(size_t offset = 0);
        virtual MappedMemory map_memory();
        virtual VertexBuffer& unmap_memory();

        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;
    };


    class ENGINE_EXPORT IndexBuffer : public PipelineBuffer
    {

    private:
        IndexBufferComponent _M_component;

    public:
        IndexBuffer();
        delete_copy_constructors(IndexBuffer);

        virtual IndexBuffer& create(const byte* data, size_t size, IndexBufferComponent component);
        IndexBuffer& update(size_t offset, const byte* data, size_t size);
        IndexBuffer& bind(size_t offset = 0);
        IndexBufferComponent component() const;

        virtual MappedMemory map_memory();
        virtual IndexBuffer& unmap_memory();

        bool serialize(BufferWriter* writer) const override;
        bool deserialize(BufferReader* reader) override;

        static size_t component_size(IndexBufferComponent component);
        size_t component_size() const;
        size_t elements_count() const;
    };


}// namespace Engine
