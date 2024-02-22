#include <vulkan_api.hpp>
#include <vulkan_descriptor_set.hpp>


namespace Engine
{
    VulkanDescriptorSet::VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout)
    {
        vk::DescriptorSetAllocateInfo info(pool, *layout);
        m_set = API->m_device.allocateDescriptorSets(info).front();
    }

    VulkanDescriptorSet& VulkanDescriptorSet::bind(vk::PipelineLayout& layout, BindingIndex set)
    {
        API->current_command_buffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout, set, 1, &m_set, 0,
                                                         nullptr);
        return *this;
    }
}// namespace Engine
