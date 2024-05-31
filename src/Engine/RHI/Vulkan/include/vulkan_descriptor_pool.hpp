#pragma once
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanDescriptorSet;
    using VulkanDescriptor = Vector<VulkanDescriptorSet>;

    struct VulkanDescriptorPool {

    private:
        struct Entry {
            uint_t m_allocated_instances = 0;
            vk::DescriptorPool m_pool;

            Entry(VulkanDescriptorPool* pool);
            ~Entry();
        };

        using Frame      = Vector<VulkanDescriptorSet>;
        using FramesList = Vector<Frame>;

        struct VulkanPipeline* m_pipeline;
        List<Entry*> m_entries;
        Vector<FramesList> m_frames_list;
        Vector<vk::DescriptorPoolSize> m_pool_sizes;
        size_t m_frame_index;
        size_t m_object_index;

        FramesList& frames_list();
        VulkanDescriptorPool& allocate_new_object();

    public:
        VulkanDescriptorPool(Vector<vk::DescriptorPoolSize>&& sizes, struct VulkanPipeline* pipeline);
        size_t max_sets_per_pool();
        VulkanDescriptorPool& next();
        VulkanDescriptorPool& reset();
        VulkanDescriptorSet* get(BindingIndex set);
        size_t descriptor_sets_per_frame() const;
        Vector<VulkanDescriptorSet>& get_sets_array();

        ~VulkanDescriptorPool();
    };
}// namespace Engine
