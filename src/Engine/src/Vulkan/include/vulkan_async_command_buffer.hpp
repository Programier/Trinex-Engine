#pragma once

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

        VulkanSyncObject(bool is_secondary);
        ~VulkanSyncObject();
    };

    struct VulkanCommandBufferState
    {
        struct VulkanFramebuffer* _M_current_framebuffer = nullptr;
        struct VulkanShader* _M_current_shader = nullptr;

        VulkanCommandBufferState& reset();
    };

    struct ThreadedCommandBuffer : VulkanSyncObject, VulkanCommandBufferState
    {
        bool _M_is_begined = false;
        bool _M_is_secondary = false;
        vk::CommandBuffer _M_buffer;
        vk::SubmitInfo _M_submit_info;

        void (*_M_callback)() = nullptr;

        ThreadedCommandBuffer(bool secondary = false);
        ThreadedCommandBuffer& begin();
        ThreadedCommandBuffer& end();
        ~ThreadedCommandBuffer();
    };

    struct CommandBufferThread
    {
        std::thread* _M_thread = nullptr;
        ThreadedCommandBuffer* _M_command_buffer = nullptr;
        bool _M_is_active = false;

        CommandBufferThread();
        ~CommandBufferThread();
    };

    struct VulkanAsyncCommandBuffer : ThreadedCommandBuffer {
        Vector<ThreadedCommandBuffer*> _M_secondary_command_buffers;
        Vector<CommandBufferThread> _M_threads;

        bool _M_async_enabled           = false;
        uint32_t _M_buffer_index        = 0;


        VulkanAsyncCommandBuffer();
        vk::CommandBuffer* get(bool force_base_framebuffer = false);
        ThreadedCommandBuffer* get_threaded_command_buffer(bool force_base_framebuffer = false);
        VulkanAsyncCommandBuffer& begin_render();
        VulkanAsyncCommandBuffer& end_render();
        VulkanAsyncCommandBuffer& next_render_thread();

        ~VulkanAsyncCommandBuffer();
    };
}// namespace Engine
