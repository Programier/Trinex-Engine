#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
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

#define implement_vertex_buffer_class(name)                                                                                      \
    implement_engine_class_default_init(name);                                                                                   \
    const byte* name::data() const                                                                                               \
    {                                                                                                                            \
        return reinterpret_cast<const byte*>(buffer.data());                                                                     \
    }                                                                                                                            \
    size_t name::size() const                                                                                                    \
    {                                                                                                                            \
        return buffer.size() * sizeof(BufferType::value_type);                                                                   \
    }                                                                                                                            \
    size_t name::element_size() const                                                                                            \
    {                                                                                                                            \
        return sizeof(ElementType);                                                                                              \
    }                                                                                                                            \
    bool name::archive_process(Archive& ar)                                                                                      \
    {                                                                                                                            \
        if (!Super::archive_process(ar))                                                                                         \
            return false;                                                                                                        \
        ar & buffer;                                                                                                             \
        return ar;                                                                                                               \
    }

    implement_vertex_buffer_class(PositionVertexBuffer);
    implement_vertex_buffer_class(TexCoordVertexBuffer);
    implement_vertex_buffer_class(ColorVertexBuffer);
    implement_vertex_buffer_class(NormalVertexBuffer);
    implement_vertex_buffer_class(TangentVertexBuffer);
    implement_vertex_buffer_class(BinormalVertexBuffer);

    PipelineBuffer& PipelineBuffer::rhi_update(size_t offset, size_t size, const byte* data)
    {
        if (m_rhi_object)
        {
            rhi_object<RHI_Buffer>()->update(offset, size, data);
        }
        return *this;
    }

    VertexBuffer::VertexBuffer() : type(RHIBufferType::Static)
    {}

    VertexBuffer& VertexBuffer::rhi_create()
    {
        size_t buffer_size = size();
        if (buffer_size > 0)
            m_rhi_object.reset(engine_instance->rhi()->create_vertex_buffer(size(), data(), type));
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


    //////////////////////////// INDEX BUFFER ////////////////////////////

    IndexBuffer& IndexBuffer::rhi_create()
    {
        m_rhi_object.reset(engine_instance->rhi()->create_index_buffer(size(), data()));
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
        m_rhi_object.reset(engine_instance->rhi()->create_ssbo(init_size, init_data));
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
