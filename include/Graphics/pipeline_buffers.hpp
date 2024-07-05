#pragma once
#include <Core/archive.hpp>
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
        VertexBuffer();
        VertexBuffer& rhi_create() override;
        VertexBuffer& rhi_bind(byte stream_index, size_t offset = 0);
        size_t elements_count() const;

        virtual const byte* data() const    = 0;
        virtual size_t size() const         = 0;
        virtual size_t element_size() const = 0;
    };

    template<typename Type>
    class TypedVertexBuffer : public VertexBuffer
    {
    public:
        using ElementType = Type;
        using BufferType  = Vector<ElementType>;

        BufferType buffer;

    public:
        const byte* data() const override
        {
            return reinterpret_cast<const byte*>(buffer.data());
        }

        size_t size() const override
        {
            return buffer.size() * sizeof(Type);
        }

        size_t element_size() const override
        {
            return sizeof(Type);
        }

        bool archive_process(Archive& ar) override
        {
            if (!Super::archive_process(ar))
                return false;
            ar & buffer;
            return ar;
        }
    };

    class ENGINE_EXPORT PositionVertexBuffer : public TypedVertexBuffer<Vector3D>
    {
        declare_class(PositionVertexBuffer, VertexBuffer);
    };

    class ENGINE_EXPORT TexCoordVertexBuffer : public TypedVertexBuffer<Vector2D>
    {
        declare_class(TexCoordVertexBuffer, VertexBuffer);
    };

    class ENGINE_EXPORT ColorVertexBuffer : public TypedVertexBuffer<ByteColor>
    {
        declare_class(ColorVertexBuffer, VertexBuffer);
    };

    class ENGINE_EXPORT NormalVertexBuffer : public TypedVertexBuffer<Vector3D>
    {
        declare_class(NormalVertexBuffer, VertexBuffer);
    };

    class ENGINE_EXPORT TangentVertexBuffer : public TypedVertexBuffer<Vector3D>
    {
        declare_class(TangentVertexBuffer, VertexBuffer);
    };

    class ENGINE_EXPORT BinormalVertexBuffer : public TypedVertexBuffer<Vector3D>
    {
        declare_class(BinormalVertexBuffer, VertexBuffer);
    };


    class ENGINE_EXPORT DynamicVertexBuffer : public VertexBuffer
    {
        declare_class(DynamicVertexBuffer, VertexBuffer);

        size_t m_allocated_size;

    public:
        DynamicVertexBuffer();
        DynamicVertexBuffer& rhi_create() override;
        DynamicVertexBuffer& rhi_shrink_to_fit();
        DynamicVertexBuffer& rhi_submit_changes(size_t offset = 0, size_t size = ~static_cast<size_t>(0));
    };


    template<typename Type>
    class TypedDynamicVertexBuffer : public DynamicVertexBuffer
    {
    public:
        using ElementType = Type;
        using BufferType  = Vector<ElementType>;

        BufferType buffer;

    public:
        const byte* data() const override
        {
            return reinterpret_cast<const byte*>(buffer.data());
        }

        size_t size() const override
        {
            return buffer.size() * sizeof(Type);
        }

        size_t element_size() const override
        {
            return sizeof(Type);
        }

        bool archive_process(Archive& ar) override
        {
            if (!Super::archive_process(ar))
                return false;
            ar & buffer;
            return ar;
        }
    };

    class ENGINE_EXPORT PositionDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3D>
    {
        declare_class(PositionDynamicVertexBuffer, DynamicVertexBuffer);
    };

    class ENGINE_EXPORT TexCoordDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector2D>
    {
        declare_class(TexCoordDynamicVertexBuffer, DynamicVertexBuffer);
    };

    class ENGINE_EXPORT ColorDynamicVertexBuffer : public TypedDynamicVertexBuffer<ByteColor>
    {
        declare_class(ColorDynamicVertexBuffer, DynamicVertexBuffer);
    };

    class ENGINE_EXPORT NormalDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3D>
    {
        declare_class(NormalDynamicVertexBuffer, DynamicVertexBuffer);
    };

    class ENGINE_EXPORT TangentDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3D>
    {
        declare_class(TangentDynamicVertexBuffer, DynamicVertexBuffer);
    };

    class ENGINE_EXPORT BinormalDynamicVertexBuffer : public TypedDynamicVertexBuffer<Vector3D>
    {
        declare_class(BinormalDynamicVertexBuffer, DynamicVertexBuffer);
    };


    ///////////////// INDEX BUFFER /////////////////

    class ENGINE_EXPORT IndexBuffer : public PipelineBuffer
    {
        declare_class(IndexBuffer, PipelineBuffer);

    public:
        using ElementType = uint32_t;
        using BufferType  = Vector<ElementType>;

    public:
        BufferType buffer;

        IndexBuffer& rhi_create() override;
        IndexBuffer& rhi_bind(size_t offset = 0);

        size_t component_size() const;
        size_t elements_count() const;
        const byte* data() const;
        size_t size() const;

        ~IndexBuffer();
        bool archive_process(Archive& ar) override;
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
