#include <vulkan_api.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>


namespace Engine
{
    VulkanDescriptorPool::Entry::Entry(VulkanDescriptorPool* pool, BindingIndex set)
    {
        vk::DescriptorPoolCreateInfo pool_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                               MAX_BINDLESS_RESOURCES, pool->m_pool_sizes[set]);
        m_pool = API->m_device.createDescriptorPool(pool_info);
        pool->m_entries[set].push_back(this);
    }

    VulkanDescriptorPool::Entry::~Entry()
    {
        API->m_device.destroyDescriptorPool(m_pool);
    }

    VulkanDescriptorSet* VulkanDescriptorPool::allocate_descriptor_set(vk::DescriptorSetLayout* layout,
                                                                       BindingIndex set)
    {
        Entry* entry = m_entries[set].empty() ? nullptr : m_entries[set].back();

        if (entry == nullptr || entry->m_allocated_instances == MAX_BINDLESS_RESOURCES)
        {
            entry = new Entry(this, set);
        }
        VulkanDescriptorSet* descriptor_set = new VulkanDescriptorSet(entry->m_pool, layout);
        ++entry->m_allocated_instances;
        return descriptor_set;
    }

    VulkanDescriptorPool& VulkanDescriptorPool::pool_sizes(const Vector<Vector<vk::DescriptorPoolSize>>& sizes)
    {
        m_pool_sizes = sizes;
        m_entries.resize(sizes.size());
        return *this;
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        for (auto& set_entry : m_entries)
        {
            for (Entry* entry : set_entry)
            {
                delete entry;
            }
        }
        m_entries.clear();
    }
}// namespace Engine
