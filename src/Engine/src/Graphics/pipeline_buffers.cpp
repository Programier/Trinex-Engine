#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Core/config.hpp>
#include <Core/engine.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/predef.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <api.hpp>


namespace Engine
{
    bool PipelineBuffer::serialize(BufferWriter* writer)
    {
        if (!ApiObject::serialize(writer))
            return false;

        if (_M_resources)
        {
            if (!writer->write(*_M_resources))
            {
                error_log("Pipeline Buffer: Failed to serialize pipeline buffer!");
                return false;
            }

            return true;
        }

        error_log("Pipeline Buffer: Cannot find resources!");
        return false;
    }

    bool PipelineBuffer::deserialize(BufferReader* reader)
    {
        if (!ApiObject::deserialize(reader))
            return false;

        auto buffer = resources(true);

        if (!reader->read(*buffer))
        {
            error_log("Pipeline Buffer: Failed to deserialize pipeline buffer!");
            return false;
        }

        return true;
    }

    register_class(Engine::VertexBuffer, Engine::ApiObject);
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
        return PipelineBuffer::serialize(writer);
    }

    bool VertexBuffer::deserialize(BufferReader* reader)
    {
        if (!PipelineBuffer::deserialize(reader))
        {
            return false;
        }

        destroy();
        create(_M_resources->data(), _M_resources->size());

        if (engine_config.delete_resources_after_load)
        {
            delete_resources();
        }

        return true;
    }


    //////////////////////////// INDEX BUFFER ////////////////////////////

    register_class(Engine::IndexBuffer, Engine::ApiObject);
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
        if (!PipelineBuffer::serialize(writer))
        {
            return false;
        }

        if (!writer->write(_M_component))
        {
            error_log("IndexBuffer: Failed to serialize component type!");
            return false;
        }

        return true;
    }

    bool IndexBuffer::deserialize(BufferReader* reader)
    {
        if (!PipelineBuffer::deserialize(reader))
        {
            return false;
        }

        bool success = reader->read(_M_component);

        if (!success)
        {
            error_log("Index Buffer: Failed to read component type!");
            return false;
        }

        destroy();
        create(_M_resources->data(), _M_resources->size(), _M_component);

        if (engine_config.delete_resources_after_load)
        {
            delete_resources();
        }

        return true;
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
