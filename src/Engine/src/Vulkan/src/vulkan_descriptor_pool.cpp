#include <vulkan_api.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>


namespace Engine
{
    VulkanDescriptorPool::Entry::Entry(VulkanDescriptorPool* pool)
    {
        vk::DescriptorPoolCreateInfo pool_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                                               MAX_BINDLESS_RESOURCES, pool->_M_pool_sizes);
        _M_pool = API->_M_device.createDescriptorPool(pool_info);
        pool->_M_entries.push_back(this);
    }

    VulkanDescriptorPool::Entry::~Entry()
    {
        API->_M_device.destroyDescriptorPool(_M_pool);
    }

    VulkanDescriptorSet* VulkanDescriptorPool::allocate_descriptor_set(vk::DescriptorSetLayout* layout)
    {

        if (_M_set_size == 0)
        {
            for (auto& size : _M_pool_sizes)
            {
                _M_set_size += size.descriptorCount * 2;
            }
        }

        Entry* entry = _M_entries.empty() ? nullptr : _M_entries.back();

        if (entry == nullptr || entry->_M_allocated_instances == MAX_BINDLESS_RESOURCES)
        {
            entry = new Entry(this);
        }
        VulkanDescriptorSet* descriptor_set = new VulkanDescriptorSet(entry->_M_pool, layout);
        ++entry->_M_allocated_instances;
        return descriptor_set;
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        for (Entry* entry : _M_entries)
        {
            delete entry;
        }

        _M_entries.clear();
    }
}// namespace Engine
