#include <Core/engine.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <api.hpp>

namespace Engine
{
    static FORCE_INLINE const byte* buffer_data(const DynamicStructInstance* buffer, DynamicStruct* uniform_struct)
    {
        return (buffer && buffer->struct_instance() == uniform_struct) ? buffer->data() : nullptr;
    }

    UniformBuffer& UniformBuffer::create(const DynamicStructInstance* buffer)
    {
        destroy();
        const byte* data = buffer_data(buffer, &uniform_struct);

        EngineInstance::instance()->api_interface()->create_uniform_buffer(_M_ID, data, uniform_struct.size());
        return *this;
    }

    UniformBuffer& UniformBuffer::update(const DynamicStructInstance* buffer, size_t offset, size_t size)
    {
        const byte* data   = buffer_data(buffer, &uniform_struct);
        size_t struct_size = uniform_struct.size();

        if (_M_ID && data && offset < struct_size)
        {
            size = std::min(size, struct_size - offset);
            EngineInstance::instance()->api_interface()->update_uniform_buffer(_M_ID, offset, data, size);
        }

        return *this;
    }

    UniformBuffer& UniformBuffer::bind(BindingIndex index, size_t offset, size_t size)
    {
        if (_M_ID)
        {
            EngineInstance::instance()->api_interface()->bind_uniform_buffer(_M_ID, index);
        }
        return *this;
    }

}// namespace Engine
