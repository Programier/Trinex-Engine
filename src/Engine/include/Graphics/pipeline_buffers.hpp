#pragma once
#include <Core/engine_types.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{
    class ENGINE_EXPORT PipelineBuffer : public RenderResource
    {
        declare_class(PipelineBuffer, RenderResource);

    public:
        virtual PipelineBuffer& rhi_update(size_t offset, size_t size, const byte* data);
    };


    ///////////////// VERTEX BUFFERS /////////////////

    class ENGINE_EXPORT VertexBuffer : public PipelineBuffer
    {
        declare_class(VertexBuffer, PipelineBuffer);

    public:
        VertexBuffer& rhi_create() override;
        VertexBuffer& rhi_bind(byte stream_index, size_t offset = 0);
        size_t elements_count() const;

        virtual const byte* data() const    = 0;
        virtual size_t size() const         = 0;
        virtual size_t element_size() const = 0;
    };


    class ENGINE_EXPORT PositionVertexBuffer : public VertexBuffer
    {
        declare_class(PositionVertexBuffer, VertexBuffer);

    public:
        using ElementType = Vector3D;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
    };

    class ENGINE_EXPORT TexCoordVertexBuffer : public VertexBuffer
    {
        declare_class(TexCoordVertexBuffer, VertexBuffer);

    public:
        using ElementType = Vector2D;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
    };

    class ENGINE_EXPORT ColorVertexBuffer : public VertexBuffer
    {
        declare_class(ColorVertexBuffer, VertexBuffer);

    public:
        using ElementType = ByteColor;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
    };

    class ENGINE_EXPORT NormalVertexBuffer : public VertexBuffer
    {
        declare_class(NormalVertexBuffer, VertexBuffer);

    public:
        using ElementType = Vector3D;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
    };

    class ENGINE_EXPORT TangentVertexBuffer : public VertexBuffer
    {
        declare_class(TangentVertexBuffer, VertexBuffer);

    public:
        using ElementType = Vector3D;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
    };

    class ENGINE_EXPORT BinormalVertexBuffer : public VertexBuffer
    {
        declare_class(BinormalVertexBuffer, VertexBuffer);

    public:
        using ElementType = Vector3D;
        using BufferType  = Vector<ElementType>;
        BufferType buffer;

        const byte* data() const override;
        size_t size() const override;
        size_t element_size() const override;
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
        IndexBuffer& rhi_create() override;
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

    class ENGINE_EXPORT SSBO : public PipelineBuffer
    {
        declare_class(SSBO, PipelineBuffer);

    public:
        size_t init_size      = 0;
        const byte* init_data = nullptr;

        SSBO& rhi_create() override;
        SSBO& rhi_bind(BindLocation location);
    };
}// namespace Engine
