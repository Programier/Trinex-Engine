#pragma once
#include <Core/api_object.hpp>
#include <Core/buffer_types.hpp>
#include <Core/engine_types.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/resource.hpp>

namespace Engine
{
    class ENGINE_EXPORT PipelineBuffer : public Resource<Buffer, ApiObjectNoBase>, SerializableObject
    {
    private:
        size_t _M_size = 0;

    public:
        inline size_t size() const
        {
            return _M_resources ? _M_resources->size() : _M_size;
        }

        bool archive_process(Archive* archive) override;
    };

    class ENGINE_EXPORT VertexBuffer : public PipelineBuffer
    {
    public:
        virtual VertexBuffer& create();
        VertexBuffer& update(size_t offset, const byte* data, size_t size);
        VertexBuffer& bind(size_t offset = 0);
        virtual MappedMemory map_memory();
        virtual VertexBuffer& unmap_memory();

        bool archive_process(Archive* archive) override;
    };


    class ENGINE_EXPORT IndexBuffer : public PipelineBuffer
    {

    private:
        IndexBufferComponent _M_component;

    public:
        IndexBuffer& create();
        IndexBuffer& update(size_t offset, const byte* data, size_t size);
        IndexBuffer& bind(size_t offset = 0);
        IndexBufferComponent component() const;
        IndexBuffer& component(IndexBufferComponent component);

        virtual MappedMemory map_memory();
        virtual IndexBuffer& unmap_memory();

        bool archive_process(Archive* archive) override;

        static size_t component_size(IndexBufferComponent component);
        size_t component_size() const;
        size_t elements_count() const;
    };


}// namespace Engine
