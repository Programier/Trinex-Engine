#pragma once

#include <vulkan_definitions.hpp>

namespace Engine
{
    struct VulkanDescriptorSet;

    struct VulkanDescriptorPool {
        struct Entry {
            uint_t m_allocated_instances = 0;
            vk::DescriptorPool m_pool;

            Entry(VulkanDescriptorPool* pool, BindingIndex set);
            ~Entry();
        };

        Vector<List<Entry*>> m_entries;
        Vector<Vector<vk::DescriptorPoolSize>> m_pool_sizes;

        VulkanDescriptorSet* allocate_descriptor_set(vk::DescriptorSetLayout* layout, BindingIndex set);
        VulkanDescriptorPool& pool_sizes(const Vector<Vector<vk::DescriptorPoolSize>>& sizes);

        ~VulkanDescriptorPool();
    };
}// namespace Engine
