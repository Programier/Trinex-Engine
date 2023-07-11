#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descriptor_set.hpp>


namespace Engine
{
    VulkanDescriptorSet::VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout)
    {
        vk::DescriptorSetAllocateInfo info(pool, *layout);
        _M_set = API->_M_device.allocateDescriptorSets(info).front();
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout)
    {
        API->_M_command_buffer->get().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, 0, 1, &_M_set, 0,
                                                         nullptr);
        return *this;
    }
}// namespace Engine
