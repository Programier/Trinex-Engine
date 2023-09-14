#pragma once

#include <vulkan_object.hpp>
#include <vulkan_definitions.hpp>

namespace Engine
{
    struct VulkanDescriptorSet;

    struct VulkanDescriptorPool : public VulkanObject {
        struct Entry {
            uint_t _M_allocated_instances = 0;
            vk::DescriptorPool _M_pool;

            Entry(VulkanDescriptorPool* pool);
            ~Entry();
        };

        List<Entry*> _M_entries;

        Vector<vk::DescriptorPoolSize> _M_pool_sizes;
        size_t _M_set_size = 0;
        VulkanDescriptorSet* allocate_descriptor_set(vk::DescriptorSetLayout* layout);

        ~VulkanDescriptorPool();
    };
}// namespace Engine
