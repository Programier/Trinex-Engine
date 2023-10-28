#pragma once
#include <Core/api_object.hpp>
#include <Core/dynamic_struct.hpp>
#include <Core/engine_types.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/resource.hpp>

namespace Engine
{

    using PositionBuffer = Vector<Vector3D>;
    using TexCoordBuffer = Vector<Vector2D>;
    using ColorBuffer    = Vector<ByteColor>;

    class ENGINE_EXPORT PipelineBuffer : public ApiObject
    {
        declare_class(PipelineBuffer, ApiObject);

    public:
        MappedMemory rhi_map_buffer();
        PipelineBuffer& rhi_unmap_buffer();
        PipelineBuffer& rhi_update(size_t offset, size_t size, const byte* data);
    };


    ///////////////// VERTEX BUFFERS /////////////////

    class ENGINE_EXPORT VertexBuffer : public PipelineBuffer
    {
        declare_class(VertexBuffer, PipelineBuffer);

    public:
        VertexBuffer& rhi_create();
        VertexBuffer& rhi_bind(byte stream_index, size_t offset = 0);

        virtual const byte* data() const = 0;
        virtual size_t size() const      = 0;
    };


    class ENGINE_EXPORT PositionVertexBuffer : public VertexBuffer
    {
        declare_class(PositionVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<Vector3D>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };

    class ENGINE_EXPORT TexCoordVertexBuffer : public VertexBuffer
    {
        declare_class(TexCoordVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<Vector2D>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };

    class ENGINE_EXPORT ColorVertexBuffer : public VertexBuffer
    {
        declare_class(ColorVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<ByteColor>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };

    class ENGINE_EXPORT NormalVertexBuffer : public VertexBuffer
    {
        declare_class(NormalVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<Vector3D>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };

    class ENGINE_EXPORT TangentVertexBuffer : public VertexBuffer
    {
        declare_class(TangentVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<Vector3D>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };

    class ENGINE_EXPORT BinormalVertexBuffer : public VertexBuffer
    {
        declare_class(BinormalVertexBuffer, VertexBuffer);

    public:
        using BufferType = Vector<Vector3D>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
    };


    ///////////////// INDEX BUFFER /////////////////

    class ENGINE_EXPORT IndexBuffer : public PipelineBuffer
    {
        declare_class(IndexBuffer, PipelineBuffer);

    public:
        using ByteBuffer  = Vector<Engine::byte>;
        using ShortBuffer = Vector<Engine::ushort_t>;
        using IntBuffer   = Vector<Engine::uint_t>;

    private:
        IndexBufferComponent _M_component;

        union
        {
            ByteBuffer* _M_byte_buffer = nullptr;
            ShortBuffer* _M_short_buffer;
            IntBuffer* _M_int_buffer;
        };

    public:
        IndexBuffer& rhi_create();
        IndexBuffer& rhi_bind(size_t offset = 0);

        IndexBuffer& setup(IndexBufferComponent component);
        IndexBufferComponent component() const;
        static size_t component_size(IndexBufferComponent component);
        size_t component_size() const;
        size_t elements_count() const;

        IndexBuffer& cleanup();
        const byte* data() const;
        size_t size() const;

        ByteBuffer* byte_buffer() const;
        ShortBuffer* short_buffer() const;
        IntBuffer* int_buffer() const;
    };

    class ENGINE_EXPORT UniformBuffer : public PipelineBuffer
    {
        declare_class(UniformBuffer, PipelineBuffer);

    public:
        size_t init_size      = 0;
        const byte* init_data = nullptr;

        UniformBuffer& rhi_create();
        UniformBuffer& bind(BindingIndex binding, BindingIndex set = 0);
    };


    class ENGINE_EXPORT SSBO : public PipelineBuffer
    {
        declare_class(SSBO, PipelineBuffer);

    public:
        size_t init_size      = 0;
        const byte* init_data = nullptr;

        SSBO& rhi_create();
        SSBO& bind(BindingIndex binding, BindingIndex set = 0);
    };
}// namespace Engine
