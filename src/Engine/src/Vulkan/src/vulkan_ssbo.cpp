#include <vulkan_api.hpp>
#include <vulkan_ssbo.hpp>

namespace Engine
{

    VulkanSSBO& VulkanSSBO::create(const byte* data, size_t size)
    {
        _M_instance_address = this;
        VulkanBufferBase::create(size, data, vk::BufferUsageFlagBits::eStorageBuffer);
        return *this;
    }

}// namespace Engine
