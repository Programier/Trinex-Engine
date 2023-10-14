#pragma once

#include <Core/engine_types.hpp>
#include <vulkan_headers.hpp>


namespace std
{
    class thread;
}

namespace Engine
{
    struct VulkanSyncObject {
        vk::Semaphore _M_available_semaphore;
        vk::Semaphore _M_finished_semaphore;
        vk::Fence _M_fence;

        VulkanSyncObject();
        VulkanSyncObject& wait();
        VulkanSyncObject& submit(vk::CommandBuffer& buffer);
        ~VulkanSyncObject();
    };


    struct VulkanCommandBuffer {
        Vector<VulkanSyncObject> _M_sync_objects;
        Vector<vk::CommandBuffer> _M_command_buffers;

        VulkanCommandBuffer();
        VulkanCommandBuffer& begin();
        VulkanCommandBuffer& end();
        vk::CommandBuffer& get();
        VulkanSyncObject& sync_object();

        operator vk::CommandBuffer&();

        template<typename Fn, typename... Args>
        VulkanCommandBuffer& exec(Fn&& fn, Args&&... args)
        {
            (get().*fn)(std::forward<Args>(args)...);
            return *this;
        }


        ~VulkanCommandBuffer();
    };
}// namespace Engine
