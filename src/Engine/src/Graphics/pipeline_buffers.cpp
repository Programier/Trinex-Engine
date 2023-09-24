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

    implement_class(PipelineBufferNoResource, "Engine");
    implement_class(PipelineBuffer, "Engine");
    implement_class(IndexBuffer, "Engine");
    implement_class(VertexBuffer, "Engine");
    implement_class(UniformBuffer, "Engine");
    implement_class(SSBO, "Engine");
    implement_default_initialize_class(PipelineBufferNoResource);
    implement_default_initialize_class(PipelineBuffer);
    implement_default_initialize_class(IndexBuffer);
    implement_default_initialize_class(VertexBuffer);
    implement_default_initialize_class(UniformBuffer);
    implement_default_initialize_class(SSBO);


    MappedMemory PipelineBufferNoResource::map_buffer()
    {
        if (_M_rhi_buffer)
        {
            return _M_rhi_buffer->map_buffer();
        }

        return {};
    }

    PipelineBufferNoResource& PipelineBufferNoResource::unmap_buffer()
    {
        if (_M_rhi_buffer)
        {
            _M_rhi_buffer->unmap_buffer();
        }
        return *this;
    }

    PipelineBufferNoResource& PipelineBufferNoResource::update(size_t offset, size_t size, const byte* data)
    {
        if (_M_rhi_buffer)
        {
            _M_rhi_buffer->update(offset, size, data);
        }
        return *this;
    }

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
                error_log("Pipeline Buffer", "Failed to process resources!");
                return false;
            }

            _M_size = _M_resources->size();
            return true;
        }

        if (archive->is_saving())
        {
            error_log("Pipeline Buffer", "Cannot find resources!");
            return false;
        }

        return static_cast<bool>(*archive);
    }


    VertexBuffer& VertexBuffer::rhi_create()
    {
        if (_M_resources == nullptr)
        {
            error_log("IndexBuffer", "Cannot create vertex buffer: No resources found!");
            return *this;
        }

        Super::rhi_create();

        Buffer* buffer = resources();

        destroy();
        _M_rhi_vertex_buffer = engine_instance->api_interface()->create_vertex_buffer(buffer->size(), buffer->data());
        return *this;
    }

    VertexBuffer& VertexBuffer::bind(byte stream_index, size_t offset)
    {
        if (_M_rhi_vertex_buffer)
        {
            _M_rhi_vertex_buffer->bind(stream_index, offset);
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
            if (engine_instance->api() != EngineAPI::NoAPI && engine_config.load_meshes_to_gpu)
                rhi_create();

            if (engine_config.delete_resources_after_load)
            {
                delete_resources();
            }
        }

        return static_cast<bool>(archive);
    }


    //////////////////////////// INDEX BUFFER ////////////////////////////

    IndexBuffer& IndexBuffer::rhi_create()
    {
        if (_M_resources == nullptr)
        {
            error_log("IndexBuffer", "Cannot create index buffer: No resources found!");
            return *this;
        }

        Super::rhi_create();
        _M_rhi_index_buffer = EngineInstance::instance()->api_interface()->create_index_buffer(
                _M_resources->size(), _M_resources->data(), _M_component);
        return *this;
    }

    IndexBuffer& IndexBuffer::bind(size_t offset)
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

    IndexBuffer& IndexBuffer::component(IndexBufferComponent component)
    {
        _M_component = component;
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
            error_log("Index Buffer", "Failed to process component type!");
            return false;
        }

        if (archive->is_reading())
        {
            destroy();
            if (engine_instance->api() != EngineAPI::NoAPI && engine_config.load_meshes_to_gpu)
                rhi_create();

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
        return PipelineBuffer::size() / component_size();
    }


    UniformBuffer& UniformBuffer::rhi_create()
    {
        Super::rhi_create();
        _M_rhi_uniform_buffer = engine_instance->api_interface()->create_uniform_buffer(init_size, init_data);
        return *this;
    }

    UniformBuffer& UniformBuffer::bind(BindingIndex binding, BindingIndex set)
    {
        if (_M_rhi_uniform_buffer)
        {
            _M_rhi_uniform_buffer->bind(binding, set);
        }
        return *this;
    }


    SSBO& SSBO::rhi_create()
    {
        Super::rhi_create();

        _M_rhi_ssbo = engine_instance->api_interface()->create_ssbo(init_size, init_data);
        return *this;
    }

    SSBO& SSBO::bind(BindingIndex binding, BindingIndex set)
    {
        if (_M_rhi_ssbo)
        {
            _M_rhi_ssbo->bind(binding, set);
        }

        return *this;
    }

    bool SSBO::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
        {
            return false;
        }

        if (archive->is_reading())
        {
            if (engine_instance->api() != EngineAPI::NoAPI && engine_config.load_meshes_to_gpu)
                rhi_create();
        }

        return static_cast<bool>(archive);
    }
}// namespace Engine
