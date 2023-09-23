#pragma once
#include <Core/api_object.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine_types.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/resource.hpp>

namespace Engine
{

    class PipelineBufferNoResource : public ApiObject
    {
        declare_class(PipelineBufferNoResource, ApiObject);

    public:
        MappedMemory map_buffer();
        PipelineBufferNoResource& unmap_buffer();
        PipelineBufferNoResource& update(size_t offset, size_t size, const byte* data);
    };

    class ENGINE_EXPORT PipelineBuffer : public Resource<Buffer, PipelineBufferNoResource>
    {
        declare_class(PipelineBuffer, PipelineBufferNoResource);

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
        declare_class(VertexBuffer, PipelineBuffer);

    public:
        VertexBuffer& rhi_create() override;
        VertexBuffer& bind(byte stream_index, size_t offset = 0);

        bool archive_process(Archive* archive) override;
    };


    class ENGINE_EXPORT IndexBuffer : public PipelineBuffer
    {
        declare_class(IndexBuffer, PipelineBuffer);

    private:
        IndexBufferComponent _M_component;

    public:
        IndexBuffer& rhi_create() override;
        IndexBuffer& bind(size_t offset = 0);
        IndexBufferComponent component() const;
        IndexBuffer& component(IndexBufferComponent component);

        bool archive_process(Archive* archive) override;

        static size_t component_size(IndexBufferComponent component);
        size_t component_size() const;
        size_t elements_count() const;
    };

    class ENGINE_EXPORT UniformStructInstance : public DynamicStructInstanceProxy, public PipelineBuffer
    {
    public:
        UniformStructInstance(DynamicStructBase* struct_instance, Index index);

        byte* data() override;
        const byte* data() const override;
        UniformStructInstance& rhi_create() override;

        UniformStructInstance& bind(BindingIndex binding, BindingIndex set = 0);
    };

    class ENGINE_EXPORT UniformStruct : public DynamicStruct<UniformStructInstance>
    {
        declare_class(UniformStruct, DynamicStructBase);
    };


    class ENGINE_EXPORT SSBO : public PipelineBufferNoResource
    {
        declare_class(SSBO, PipelineBufferNoResource);

    public:
        size_t init_size;
        const byte* init_data = nullptr;

        SSBO& rhi_create() override;
        SSBO& bind(BindingIndex binding, BindingIndex set = 0);

        bool archive_process(Archive* archive) override;
    };
}// namespace Engine
