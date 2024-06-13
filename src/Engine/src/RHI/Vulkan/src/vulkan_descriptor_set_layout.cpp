#include <vulkan_api.hpp>
#include <vulkan_descript_set_layout.hpp>

namespace Engine
{
    void VulkanDescriptorSetLayout::destroy()
    {
        for (auto& layout : layouts)
        {
            DESTROY_CALL(destroyDescriptorSetLayout, layout);
        }

        layouts.clear();
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
    {
        destroy();
    }
}// namespace Engine
