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

    bool PipelineBuffer::archive_process(Archive* archive)
    {
        if (!SerializableObject::archive_process(archive))
            return false;

        if (archive->is_reading())
        {
            resources(true);
        }

        if (_M_resources)
        {
            if (!((*archive) & *_M_resources))
            {
                error_log("Pipeline Buffer: Failed to process resources!");
                return false;
            }

            return true;
        }

        if (archive->is_saving())
        {
            error_log("Pipeline Buffer: Cannot find resources!");
            return false;
        }

        return static_cast<bool>(*archive);
    }


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

    bool VertexBuffer::archive_process(Archive* archive)
    {
        if (!PipelineBuffer::archive_process(archive))
        {
            return false;
        }

        if (archive->is_reading())
        {
            destroy();
            create(_M_resources->data(), _M_resources->size());

            if (engine_config.delete_resources_after_load)
            {
                delete_resources();
            }
        }

        return static_cast<bool>(archive);
    }


    //////////////////////////// INDEX BUFFER ////////////////////////////


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

    bool IndexBuffer::archive_process(Archive* archive)
    {
        if (!PipelineBuffer::archive_process(archive))
        {
            return false;
        }

        if (!((*archive) & _M_component))
        {
            error_log("Index Buffer: Failed to process component type!");
            return false;
        }

        if (archive->is_reading())
        {
            destroy();
            create(_M_resources->data(), _M_resources->size(), _M_component);

            if (engine_config.delete_resources_after_load)
            {
                delete_resources();
            }
        }

        return static_cast<bool>(archive);
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
