#include <vulkan_api.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_pipeline.hpp>

namespace Engine
{
    VulkanDescriptorPool::Entry::Entry(VulkanDescriptorPool* pool)
    {
        size_t max_sets = MAX_BINDLESS_RESOURCES * API->m_framebuffers_count * pool->descriptor_sets_per_frame();
        vk::DescriptorPoolCreateInfo pool_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, max_sets,
                                               pool->m_pool_sizes);
        m_pool = API->m_device.createDescriptorPool(pool_info);
        pool->m_entries.push_back(this);
    }

    VulkanDescriptorPool::Entry::~Entry()
    {
        API->m_device.destroyDescriptorPool(m_pool);
    }

    VulkanDescriptorPool::VulkanDescriptorPool(Vector<vk::DescriptorPoolSize>&& sizes, struct VulkanPipeline* pipeline)
        : m_pipeline(pipeline), m_pool_sizes(std::move(sizes)), m_frame_index(0), m_object_index(0)
    {
        m_frames_list.resize(API->m_framebuffers_count);
        allocate_new_object();
    }

    VulkanDescriptorPool::FramesList& VulkanDescriptorPool::frames_list()
    {
        return m_frames_list[API->m_current_buffer];
    }

    VulkanDescriptorPool& VulkanDescriptorPool::allocate_new_object()
    {
        if (m_entries.empty() || m_entries.back()->m_allocated_instances >= MAX_BINDLESS_RESOURCES)
        {
            new Entry(this);
        }

        Entry* entry = m_entries.back();
        ++entry->m_allocated_instances;
        auto& pool = entry->m_pool;
        for (auto& frame_list : m_frames_list)
        {
            frame_list.emplace_back();
            uint_t set_index = 0;
            frame_list.back().resize(m_pipeline->descriptor_sets_count());
            for (auto& set : frame_list.back())
            {
                set.init(pool, &m_pipeline->m_descriptor_set_layout[set_index]);
                ++set_index;
            }
        }

        return *this;
    }

    VulkanDescriptorPool& VulkanDescriptorPool::next()
    {
        ++m_object_index;

        if (frames_list().size() <= m_object_index)
        {
            allocate_new_object();
        }

        return *this;
    }

    VulkanDescriptorPool& VulkanDescriptorPool::reset()
    {
        if (m_frame_index != API->m_current_frame)
        {
            m_frame_index  = API->m_current_frame;
            m_object_index = 0;
        }
        return *this;
    }

    VulkanDescriptorSet* VulkanDescriptorPool::get(BindingIndex set)
    {
        return get_sets_array().data() + set;
    }

    size_t VulkanDescriptorPool::descriptor_sets_per_frame() const
    {
        return m_pipeline->descriptor_sets_count();
    }

    VulkanDescriptorPool::Frame& VulkanDescriptorPool::get_sets_array()
    {
        return frames_list()[m_object_index];
    }

    VulkanDescriptorPool::~VulkanDescriptorPool()
    {
        for (auto& pool_entry : m_entries)
        {
            delete pool_entry;
        }

        m_entries.clear();
    }
}// namespace Engine
