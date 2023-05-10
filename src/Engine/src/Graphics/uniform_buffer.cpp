#include <Core/engine.hpp>
#include <Graphics/uniform_buffer.hpp>
#include <api.hpp>

namespace Engine
{
    UniformBufferBase& UniformBufferBase::create_buffer(const byte* data, size_t size)
    {
        destroy();
        EngineInstance::instance()->api_interface()->create_uniform_buffer(_M_ID, data, size);
        return *this;
    }

    UniformBufferBase& UniformBufferBase::update_buffer(size_t offset, const byte* data, size_t size)
    {
        if (_M_ID)
            EngineInstance::instance()->api_interface()->update_uniform_buffer(_M_ID, offset, data, size);
        return *this;
    }

    UniformBufferBase& UniformBufferBase::bind_buffer(BindingIndex index, size_t offset, size_t size)
    {
        auto ubo_size = buffer_size();
        if (offset < ubo_size && _M_ID)
        {
            size = glm::min(size, ubo_size - offset);
            EngineInstance::instance()->api_interface()->bind_uniform_buffer(_M_ID, index, offset, size);
        }
        return *this;
    }

}// namespace Engine
