#pragma once

#include <vulkan_definitions.hpp>

namespace Engine
{
    struct VulkanDescriptorSet;

    struct VulkanDescriptorPool {
        struct Entry {
            uint_t _M_allocated_instances = 0;
            vk::DescriptorPool _M_pool;

            Entry(VulkanDescriptorPool* pool, BindingIndex set);
            ~Entry();
        };

        Vector<List<Entry*>> _M_entries;
        Vector<Vector<vk::DescriptorPoolSize>> _M_pool_sizes;

        VulkanDescriptorSet* allocate_descriptor_set(vk::DescriptorSetLayout* layout, BindingIndex set);
        VulkanDescriptorPool& pool_sizes(const Vector<Vector<vk::DescriptorPoolSize>>& sizes);

        ~VulkanDescriptorPool();
    };
}// namespace Engine
