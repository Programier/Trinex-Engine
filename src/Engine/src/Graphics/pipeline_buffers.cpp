#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>


namespace Engine
{

    implement_engine_class_default_init(PipelineBuffer);
    implement_engine_class_default_init(IndexBuffer);
    implement_engine_class_default_init(VertexBuffer);
    implement_engine_class_default_init(SSBO);


    PipelineBuffer& PipelineBuffer::rhi_update(size_t offset, size_t size, const byte* data)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_Buffer>()->update(offset, size, data);
        }
        return *this;
    }

    VertexBuffer::VertexBuffer()
    {}

    VertexBuffer& VertexBuffer::rhi_create()
    {
        size_t buffer_size = size();
        if (buffer_size > 0)
            m_rhi_object.reset(rhi->create_vertex_buffer(buffer_size, data(), RHIBufferType::Static));
        return *this;
    }

    VertexBuffer& VertexBuffer::rhi_bind(byte stream_index, size_t offset)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_VertexBuffer>()->bind(stream_index, offset);
        }

        return *this;
    }

    size_t VertexBuffer::elements_count() const
    {
        return size() / element_size();
    }

    implement_engine_class_default_init(PositionVertexBuffer);
    implement_engine_class_default_init(TexCoordVertexBuffer);
    implement_engine_class_default_init(ColorVertexBuffer);
    implement_engine_class_default_init(NormalVertexBuffer);
    implement_engine_class_default_init(TangentVertexBuffer);
    implement_engine_class_default_init(BinormalVertexBuffer);

    implement_engine_class_default_init(DynamicVertexBuffer);
    DynamicVertexBuffer::DynamicVertexBuffer() : m_allocated_size(0)
    {}

    DynamicVertexBuffer& DynamicVertexBuffer::rhi_create()
    {
        Vector<int> test;
        test.shrink_to_fit();
        m_allocated_size = size();
        if (m_allocated_size > 0)
            m_rhi_object.reset(rhi->create_vertex_buffer(m_allocated_size, data(), RHIBufferType::Dynamic));
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

    implement_engine_class_default_init(PositionDynamicVertexBuffer);
    implement_engine_class_default_init(TexCoordDynamicVertexBuffer);
    implement_engine_class_default_init(ColorDynamicVertexBuffer);
    implement_engine_class_default_init(NormalDynamicVertexBuffer);
    implement_engine_class_default_init(TangentDynamicVertexBuffer);
    implement_engine_class_default_init(BinormalDynamicVertexBuffer);

    //////////////////////////// INDEX BUFFER ////////////////////////////

    IndexBuffer& IndexBuffer::rhi_create()
    {
        m_rhi_object.reset(rhi->create_index_buffer(size(), data()));
        return *this;
    }

    const byte* IndexBuffer::data() const
    {
        return reinterpret_cast<const byte*>(buffer.data());
    }

    size_t IndexBuffer::size() const
    {
        return buffer.size() * sizeof(uint32_t);
    }

    IndexBuffer::~IndexBuffer()
    {}

    bool IndexBuffer::archive_process(Archive& ar)
    {
        if (!Super::archive_process(ar))
            return false;
        ar & buffer;
        return ar;
    }

    IndexBuffer& IndexBuffer::rhi_bind(size_t offset)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_IndexBuffer>()->bind(offset);
        }
        return *this;
    }

    size_t IndexBuffer::component_size() const
    {
        return sizeof(uint32_t);
    }

    size_t IndexBuffer::elements_count() const
    {
        return buffer.size();
    }

    SSBO& SSBO::rhi_create()
    {
        m_rhi_object.reset(rhi->create_ssbo(init_size, init_data));
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
