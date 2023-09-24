#include <vulkan_api.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>


namespace Engine
{
    VulkanDescriptorPool::Entry::Entry(VulkanDescriptorPool* pool, BindingIndex set)
    {
        vk::DescriptorPoolCreateInfo pool_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                               MAX_BINDLESS_RESOURCES, pool->_M_pool_sizes[set]);
        _M_pool = API->_M_device.createDescriptorPool(pool_info);
        pool->_M_entries[set].push_back(this);
    }

    VulkanDescriptorPool::Entry::~Entry()
    {
        API->_M_device.destroyDescriptorPool(_M_pool);
    }

    VulkanDescriptorSet* VulkanDescriptorPool::allocate_descriptor_set(vk::DescriptorSetLayout* layout,
                                                                       BindingIndex set)
    {
        Entry* entry = _M_entries[set].empty() ? nullptr : _M_entries[set].back();

        if (entry == nullptr || entry->_M_allocated_instances == MAX_BINDLESS_RESOURCES)
        {
            entry = new Entry(this, set);
        }
        VulkanDescriptorSet* descriptor_set = new VulkanDescriptorSet(entry->_M_pool, layout);
        ++entry->_M_allocated_instances;
        return descriptor_set;
    }

    VulkanDescriptorPool& VulkanDescriptorPool::pool_sizes(const Vector<Vector<vk::DescriptorPoolSize>>& sizes)
    {
        _M_pool_sizes = sizes;
        _M_entries.resize(sizes.size());
        return *this;
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        for (auto& set_entry : _M_entries)
        {
            for (Entry* entry : set_entry)
            {
                delete entry;
            }
        }
        _M_entries.clear();
    }
}// namespace Engine
