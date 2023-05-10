#pragma once

#include <Core/api_object.hpp>
#include <Core/engine_types.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class ENGINE_EXPORT UniformBufferBase : public ApiObject
    {
    protected:
        UniformBufferBase& create_buffer(const byte* data, size_t size);
        UniformBufferBase& update_buffer(size_t offset, const byte* data, size_t size);
        virtual size_t buffer_size() = 0;

    public:
        UniformBufferBase& bind_buffer(BindingIndex index, size_t offset = 0, size_t size = Constants::max_size);
    };


    template<typename Type>
    class UniformBuffer : public UniformBufferBase
    {
    protected:

        size_t buffer_size() override
        {
            return sizeof(Type);
        }

    public:
        Type buffer;

        UniformBuffer& create()
        {
            UniformBufferBase::create_buffer(reinterpret_cast<const byte*>(&buffer), sizeof(buffer));
            return *this;
        }

        UniformBuffer& update(size_t offset, size_t size)
        {
            if(offset < sizeof(Type))
            {
                size = glm::min(sizeof(Type) - offset, size);
                UniformBufferBase::update_buffer(offset, reinterpret_cast<const byte*>(&buffer) + offset, size);
            }

            return *this;
        }
    };
}// namespace Engine
