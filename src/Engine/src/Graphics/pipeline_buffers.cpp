#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/predef.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <api.hpp>


namespace Engine
{

    REGISTER_CLASS(Engine::VertexBuffer, Engine::ApiObject);
    VertexBuffer::VertexBuffer()
    {}

    VertexBuffer& VertexBuffer::create(const byte* data, size_t size)
    {
        destroy();
        EngineInstance::instance()->api_interface()->create_vertex_buffer(_M_ID, data, size);
        _M_size = size;
        return *this;
    }

    VertexBuffer& VertexBuffer::update(size_t offset, const byte* data, size_t size)
    {
        EngineInstance::instance()->api_interface()->update_vertex_buffer(_M_ID, offset, data, size);
        return *this;
    }

    VertexBuffer& VertexBuffer::bind(size_t offset)
    {
        EngineInstance::instance()->api_interface()->bind_vertex_buffer(_M_ID, offset);
        return *this;
    }

    MappedMemory VertexBuffer::map_memory()
    {
        if (_M_ID)
        {
            return EngineInstance::instance()->api_interface()->map_vertex_buffer(_M_ID);
        }

        return MappedMemory(nullptr, 0);
    }

    VertexBuffer& VertexBuffer::unmap_memory()
    {
        if (_M_ID)
        {
            EngineInstance::instance()->api_interface()->unmap_vertex_buffer(_M_ID);
        }
        return *this;
    }

    bool VertexBuffer::serialize(BufferWriter* writer)
    {
        if (_M_ID)
        {
            if (!ApiObject::serialize(writer))
            {
                return false;
            }

            MappedMemory memory = map_memory();
            bool success        = writer->write(memory);
            if (!success)
            {
                logger->error("Vertex Buffer: Failed to serialize vertex buffer");
            }

            unmap_memory();
            return success;
        }
        else
        {
            logger->error("Vertex Buffer: Failed to serialize object. Object is empty!");
        }
        return false;
    }

    bool VertexBuffer::deserialize(BufferReader* reader)
    {
        if (!ApiObject::deserialize(reader))
        {
            return false;
        }

        destroy();

        size_t size = reader->read_value<size_t>();
        create(nullptr, size);

        MappedMemory memory = map_memory();
        check(memory.size() != 0);

        bool success = reader->read(memory.data(), size);
        unmap_memory();

        if (!success)
        {
            logger->error("Vertex Buffer: Failed to deserialize vertex buffer '%s'");
        }
        return success;
    }


    //////////////////////////// INDEX BUFFER ////////////////////////////

    REGISTER_CLASS(Engine::IndexBuffer, Engine::ApiObject);
    IndexBuffer::IndexBuffer()
    {}

    IndexBuffer& IndexBuffer::create(const byte* data, size_t size, IndexBufferComponent component)
    {
        destroy();
        EngineInstance::instance()->api_interface()->create_index_buffer(_M_ID, data, size, component);
        _M_component = component;
        _M_size      = size;
        return *this;
    }

    IndexBuffer& IndexBuffer::update(size_t offset, const byte* data, size_t size)
    {
        EngineInstance::instance()->api_interface()->update_index_buffer(_M_ID, offset, data, size);
        return *this;
    }

    IndexBuffer& IndexBuffer::bind(size_t offset)
    {
        EngineInstance::instance()->api_interface()->bind_index_buffer(_M_ID, offset);
        return *this;
    }

    IndexBufferComponent IndexBuffer::component() const
    {
        return _M_component;
    }

    MappedMemory IndexBuffer::map_memory()
    {
        if (_M_ID == 0)
            return MappedMemory(nullptr, 0);
        return EngineInstance::instance()->api_interface()->map_index_buffer(_M_ID);
    }

    IndexBuffer& IndexBuffer::unmap_memory()
    {
        if (_M_ID != 0)
            EngineInstance::instance()->api_interface()->unmap_index_buffer(_M_ID);
        return *this;
    }

    bool IndexBuffer::serialize(BufferWriter* writer)
    {
        if (_M_ID)
        {
            if (ApiObject::serialize(writer) == false)
                return false;

            MappedMemory memory = map_memory();
            bool success        = writer->write(_M_component) && writer->write(memory);

            if (!success)
            {
                logger->error("Index Buffer: Failed to serialize index buffer");
            }

            unmap_memory();
            return success;
        }
        else
        {
            logger->error("Index Buffer: Failed to serialize object. Object is empty!");
        }
        return false;
    }

    bool IndexBuffer::deserialize(BufferReader* reader)
    {
        if (!ApiObject::deserialize(reader))
        {
            return false;
        }

        destroy();

        bool success = reader->read(_M_component);

        if (success)
        {
            size_t size = reader->read_value<size_t>();
            create(nullptr, size, _M_component);
            MappedMemory memory = map_memory();
            check(memory.size() != 0);
            success = reader->read(memory.data(), size);
            unmap_memory();
        }

        if (!success)
        {
            logger->error("Index Buffer: Failed to deserialize index buffer");
        }
        return success;
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
        return _M_size / component_size();
    }
}// namespace Engine
