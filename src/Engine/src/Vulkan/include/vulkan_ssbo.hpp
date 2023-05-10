#pragma once
#include <vulkan_buffer.hpp>

namespace Engine
{
       struct VulkanSSBO : public VulkanBufferBase
       {
            VulkanSSBO& create(const byte* data, size_t size);
            VulkanSSBO& bind(BindingIndex index);
       };
}
