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
    implement_engine_class_default_init(UniformBuffer);
    implement_engine_class_default_init(SSBO);

#define implement_vertex_buffer_class(name)                                                                            \
    implement_engine_class_default_init(name);                                                                         \
    const byte* name::data() const                                                                                     \
    {                                                                                                                  \
        return reinterpret_cast<const byte*>(buffer.data());                                                           \
    }                                                                                                                  \
    size_t name::size() const                                                                                          \
    {                                                                                                                  \
        return buffer.size() * sizeof(BufferType::value_type);                                                         \
    }

    implement_vertex_buffer_class(PositionVertexBuffer);
    implement_vertex_buffer_class(TexCoordVertexBuffer);
    implement_vertex_buffer_class(ColorVertexBuffer);
    implement_vertex_buffer_class(NormalVertexBuffer);
    implement_vertex_buffer_class(TangentVertexBuffer);
    implement_vertex_buffer_class(BinormalVertexBuffer);

    PipelineBuffer& PipelineBuffer::rhi_update(size_t offset, size_t size, const byte* data)
    {
        if (_M_rhi_buffer)
        {
            _M_rhi_buffer->update(offset, size, data);
        }
        return *this;
    }

    VertexBuffer& VertexBuffer::rhi_create()
    {
        rhi_destroy();
        _M_rhi_vertex_buffer = engine_instance->rhi()->create_vertex_buffer(size(), data());
        return *this;
    }

    VertexBuffer& VertexBuffer::rhi_bind(byte stream_index, size_t offset)
    {
        if (_M_rhi_vertex_buffer)
        {
            _M_rhi_vertex_buffer->bind(stream_index, offset);
        }

        return *this;
    }


    //////////////////////////// INDEX BUFFER ////////////////////////////

    IndexBuffer& IndexBuffer::rhi_create()
    {
        rhi_destroy();
        _M_rhi_index_buffer = engine_instance->rhi()->create_index_buffer(size(), data(), component());
        return *this;
    }

    IndexBuffer& IndexBuffer::setup(IndexBufferComponent component)
    {
        cleanup();

        _M_component = component;

        if (_M_component == IndexBufferComponent::UnsignedByte)
            _M_byte_buffer = new ByteBuffer();
        else if (_M_component == IndexBufferComponent::UnsignedShort)
            _M_short_buffer = new ShortBuffer();
        else if (_M_component == IndexBufferComponent::UnsignedInt)
            _M_int_buffer = new IntBuffer();
        return *this;
    }

    const byte* IndexBuffer::data() const
    {
        if (_M_byte_buffer == nullptr)
            return nullptr;

        if (_M_component == IndexBufferComponent::UnsignedByte)
            return _M_byte_buffer->data();
        else if (_M_component == IndexBufferComponent::UnsignedShort)
            return reinterpret_cast<const byte*>(_M_short_buffer->data());
        else if (_M_component == IndexBufferComponent::UnsignedInt)
            return reinterpret_cast<const byte*>(_M_int_buffer->data());

        return nullptr;
    }

    size_t IndexBuffer::size() const
    {
        if (_M_byte_buffer == nullptr)
            return 0;

        if (_M_component == IndexBufferComponent::UnsignedByte)
            return _M_byte_buffer->size();
        else if (_M_component == IndexBufferComponent::UnsignedShort)
            return _M_short_buffer->size() * sizeof(ushort_t);
        else if (_M_component == IndexBufferComponent::UnsignedInt)
            return _M_int_buffer->size() * sizeof(uint_t);

        return 0;
    }

    IndexBuffer::ByteBuffer* IndexBuffer::byte_buffer() const
    {
        return _M_component == IndexBufferComponent::UnsignedByte ? _M_byte_buffer : nullptr;
    }

    IndexBuffer::ShortBuffer* IndexBuffer::short_buffer() const
    {
        return _M_component == IndexBufferComponent::UnsignedShort ? _M_short_buffer : nullptr;
    }

    IndexBuffer::IntBuffer* IndexBuffer::int_buffer() const
    {
        return _M_component == IndexBufferComponent::UnsignedInt ? _M_int_buffer : nullptr;
    }

    IndexBuffer& IndexBuffer::cleanup()
    {
        if (_M_byte_buffer == nullptr)
            return *this;

        if (_M_component == IndexBufferComponent::UnsignedByte)
            delete _M_byte_buffer;
        else if (_M_component == IndexBufferComponent::UnsignedShort)
            delete _M_short_buffer;
        else if (_M_component == IndexBufferComponent::UnsignedInt)
            delete _M_int_buffer;

        _M_byte_buffer = nullptr;
        return *this;
    }

    IndexBuffer& IndexBuffer::rhi_bind(size_t offset)
    {
        if (_M_rhi_index_buffer)
        {
            _M_rhi_index_buffer->bind(offset);
        }
        return *this;
    }

    IndexBufferComponent IndexBuffer::component() const
    {
        return _M_component;
    }

    size_t IndexBuffer::component_size(IndexBufferComponent component)
    {
        switch (component)
        {
            case IndexBufferComponent::UnsignedByte:
                return sizeof(byte);
            case IndexBufferComponent::UnsignedShort:
                return sizeof(ushort_t);
            case IndexBufferComponent::UnsignedInt:
                return sizeof(uint_t);

            default:
                throw EngineException("Index Buffer: Undefined type of component!");
        }
    }

    size_t IndexBuffer::component_size() const
    {
        return component_size(_M_component);
    }

    size_t IndexBuffer::elements_count() const
    {
        return size() / component_size();
    }


    UniformBuffer& UniformBuffer::rhi_create()
    {
        rhi_destroy();
        _M_rhi_uniform_buffer = engine_instance->rhi()->create_uniform_buffer(init_size, init_data);
        return *this;
    }

    UniformBuffer& UniformBuffer::rhi_bind(BindLocation location)
    {
        if (_M_rhi_uniform_buffer)
        {
            _M_rhi_uniform_buffer->bind(location);
        }
        return *this;
    }


    SSBO& SSBO::rhi_create()
    {
        rhi_destroy();
        _M_rhi_ssbo = engine_instance->rhi()->create_ssbo(init_size, init_data);
        return *this;
    }

    SSBO& SSBO::rhi_bind(BindLocation location)
    {
        if (_M_rhi_ssbo)
        {
            _M_rhi_ssbo->bind(location);
        }

        return *this;
    }
}// namespace Engine
