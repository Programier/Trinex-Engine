#pragma once

#include <Core/engine_types.hpp>
#include <condition_variable>
#include <thread>
#include <vulkan/vulkan.hpp>


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

    struct VulkanCommandBufferState {
        struct VulkanFramebuffer* _M_current_framebuffer    = nullptr;
        struct VulkanShader* _M_current_shader              = nullptr;
        struct VulkanVertexBuffer* _M_current_vertex_buffer = nullptr;
        struct VulkanIndexBuffer* _M_current_index_buffer   = nullptr;

        VulkanCommandBufferState& reset();
    };


    struct VulkanCommandBufferThreadedEndCommand {

        std::thread* _M_thread = nullptr;
        std::mutex _M_mutex;
        std::condition_variable _M_cv;
        vk::CommandBuffer* _M_buffer     = nullptr;
        VulkanSyncObject* _M_sync_object = nullptr;
        std::uint32_t _M_swapchain_index;

        bool _M_destruct = false;

        VulkanCommandBufferThreadedEndCommand(vk::CommandBuffer* buffer, VulkanSyncObject* sync);

        static void thread_loop(VulkanCommandBufferThreadedEndCommand*);
        void execute();
        void notify();
        void wait();

        ~VulkanCommandBufferThreadedEndCommand();
    };

    struct VulkanCommandBuffer {
        Vector<VulkanCommandBufferState> _M_command_buffer_state;
        Vector<VulkanSyncObject> _M_sync_objects;
        Vector<vk::CommandBuffer> _M_command_buffers;
        Vector<VulkanCommandBufferThreadedEndCommand*> _M_end_commands;

        VulkanCommandBuffer();
        VulkanCommandBuffer& begin();
        VulkanCommandBuffer& end();
        vk::CommandBuffer& get();
        VulkanCommandBufferState& state();
        VulkanSyncObject& sync_object();
        VulkanCommandBufferThreadedEndCommand* end_command();

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
