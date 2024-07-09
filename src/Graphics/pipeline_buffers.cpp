#include <Core/archive.hpp>
#include <Core/base_engine.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{

    implement_engine_class_default_init(PipelineBuffer, 0);
    implement_engine_class_default_init(VertexBuffer, 0);
    implement_engine_class_default_init(SSBO, 0);


    PipelineBuffer& PipelineBuffer::rhi_update(size_t offset, size_t size, const byte* data)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_Buffer>()->update(offset, size, data);
        }
        return *this;
    }


    size_t PipelineBuffer::elements_count() const
    {
        return size() / element_size();
    }

    VertexBuffer::VertexBuffer()
    {}

    VertexBuffer& VertexBuffer::rhi_create()
    {
        size_t buffer_size = size();
        if (buffer_size > 0)
            m_rhi_object.reset(rhi->create_vertex_buffer(buffer_size, data(), buffer_type()));
        return *this;
    }

    VertexBuffer& VertexBuffer::rhi_bind(byte stream_index, size_t offset)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_VertexBuffer>()->bind(stream_index, element_size(), offset);
        }

        return *this;
    }


    RHIBufferType VertexBuffer::buffer_type() const
    {
        return RHIBufferType::Static;
    }

    implement_engine_class_default_init(PositionVertexBuffer, 0);
    implement_engine_class_default_init(TexCoordVertexBuffer, 0);
    implement_engine_class_default_init(ColorVertexBuffer, 0);
    implement_engine_class_default_init(NormalVertexBuffer, 0);
    implement_engine_class_default_init(TangentVertexBuffer, 0);
    implement_engine_class_default_init(BinormalVertexBuffer, 0);
    implement_engine_class_default_init(DynamicVertexBuffer, 0);

    DynamicVertexBuffer::DynamicVertexBuffer() : m_allocated_size(0)
    {}

    DynamicVertexBuffer& DynamicVertexBuffer::rhi_create()
    {
        m_allocated_size = size();
        if (m_allocated_size > 0)
            m_rhi_object.reset(rhi->create_vertex_buffer(m_allocated_size, data(), buffer_type()));
        return *this;
    }

    DynamicVertexBuffer& DynamicVertexBuffer::rhi_shrink_to_fit()
    {
        rhi_create();
        return *this;
    }

    DynamicVertexBuffer& DynamicVertexBuffer::rhi_submit_changes(size_t submit_offset, size_t submit_size)
    {
        size_t buffer_size = size();

        if (buffer_size > m_allocated_size)
        {
            rhi_create();
        }
        else
        {
            submit_offset = glm::min(submit_offset, buffer_size);
            submit_size   = glm::min(submit_size, buffer_size - submit_offset);
            rhi_update(submit_offset, submit_size, data() + submit_offset);
        }
        return *this;
    }

    RHIBufferType DynamicVertexBuffer::buffer_type() const
    {
        return RHIBufferType::Dynamic;
    }

    implement_engine_class_default_init(PositionDynamicVertexBuffer, 0);
    implement_engine_class_default_init(TexCoordDynamicVertexBuffer, 0);
    implement_engine_class_default_init(ColorDynamicVertexBuffer, 0);
    implement_engine_class_default_init(NormalDynamicVertexBuffer, 0);
    implement_engine_class_default_init(TangentDynamicVertexBuffer, 0);
    implement_engine_class_default_init(BinormalDynamicVertexBuffer, 0);

    //////////////////////////// INDEX BUFFER ////////////////////////////

    implement_engine_class_default_init(IndexBuffer, 0);
    implement_engine_class_default_init(DynamicIndexBuffer, 0);
    implement_engine_class_default_init(BufferedIndexBuffer, 0);
    implement_engine_class_default_init(BufferedDynamicIndexBuffer, 0);

    IndexBuffer& IndexBuffer::rhi_create()
    {
        m_rhi_object.reset(rhi->create_index_buffer(size(), data(), buffer_type()));
        return *this;
    }

    IndexBuffer& IndexBuffer::rhi_bind(size_t offset)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_IndexBuffer>()->bind(offset);
        }
        return *this;
    }

    RHIBufferType IndexBuffer::buffer_type() const
    {
        return RHIBufferType::Static;
    }

    size_t IndexBuffer::element_size() const
    {
        return sizeof(uint32_t);
    }


    RHIBufferType DynamicIndexBuffer::buffer_type() const
    {
        return RHIBufferType::Dynamic;
    }

    const byte* BufferedIndexBuffer::data() const
    {
        return reinterpret_cast<const byte*>(buffer.data());
    }

    size_t BufferedIndexBuffer::size() const
    {
        return buffer.size() * element_size();
    }

    bool BufferedIndexBuffer::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;
        ar & buffer;
        return ar;
    }

    const byte* BufferedDynamicIndexBuffer::data() const
    {
        return reinterpret_cast<const byte*>(buffer.data());
    }

    size_t BufferedDynamicIndexBuffer::size() const
    {
        return buffer.size() * element_size();
    }

    bool BufferedDynamicIndexBuffer::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;
        ar & buffer;
        return ar;
    }

    SSBO& SSBO::rhi_create()
    {
        m_rhi_object.reset(rhi->create_ssbo(init_size, init_data, RHIBufferType::Static));
        return *this;
    }

    SSBO& SSBO::rhi_bind(BindLocation location)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_SSBO>()->bind(location);
        }

        return *this;
    }
}// namespace Engine
