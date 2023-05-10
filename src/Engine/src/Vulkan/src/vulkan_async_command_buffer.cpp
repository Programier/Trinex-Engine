#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>


namespace Engine
{
    extern vk::Device* vulkan_device();
    extern vk::CommandPool* vulkan_command_pool();

    static const std::uint32_t max_thread_count = std::thread::hardware_concurrency();


    static void recreate_swapchain_callback()
    {
        API->_M_need_update_image_index = true;
        API->recreate_swap_chain();
    }

    static void command_buffer_error()
    {
        throw std::runtime_error("Vulkan API: Failed to execute command buffer!");
    }

    VulkanSyncObject::VulkanSyncObject(bool is_secondary)
    {
        if (!is_secondary)
        {
            _M_available_semaphore = vulkan_device()->createSemaphore(vk::SemaphoreCreateInfo());
            _M_finished_semaphore  = vulkan_device()->createSemaphore(vk::SemaphoreCreateInfo());
        }

        _M_fence = vulkan_device()->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    VulkanSyncObject::~VulkanSyncObject()
    {
        DESTROY_CALL(destroySemaphore, _M_available_semaphore);
        DESTROY_CALL(destroySemaphore, _M_finished_semaphore);
        DESTROY_CALL(destroyFence, _M_fence);
    }

    VulkanCommandBufferState& VulkanCommandBufferState::reset()
    {
        _M_current_framebuffer = nullptr;
        _M_current_shader      = nullptr;
        return *this;
    }

    ThreadedCommandBuffer::ThreadedCommandBuffer(bool is_secondary) : VulkanSyncObject(is_secondary)
    {
        _M_is_secondary = is_secondary;
        _M_buffer       = vulkan_device()->allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(*vulkan_command_pool(), vk::CommandBufferLevel::ePrimary, 1))[0];
        _M_callback = command_buffer_error;

        if (is_secondary)
        {
            _M_submit_info = vk::SubmitInfo({}, {}, _M_buffer);
        }
        else
        {
            static const vk::PipelineStageFlags wait_flags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            _M_submit_info = vk::SubmitInfo(_M_available_semaphore, wait_flags, _M_buffer, _M_finished_semaphore);
        }
    }

    ThreadedCommandBuffer& ThreadedCommandBuffer::begin()
    {
        if (!_M_is_begined)
        {
            while (vk::Result::eTimeout == API->_M_device.waitForFences(_M_fence, VK_TRUE, UINT32_MAX))
                ;
            API->_M_device.resetFences(_M_fence);


            auto current_buffer_index = API->swapchain_image_index();

            if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
            {
                _M_callback();
                return begin();
            }
            else if (current_buffer_index.result != vk::Result::eSuccess &&
                     current_buffer_index.result != vk::Result::eSuboptimalKHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }

            _M_buffer.reset();
            _M_buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            _M_is_begined = true;
        }

        return *this;
    }

    ThreadedCommandBuffer& ThreadedCommandBuffer::end()
    {
        if (_M_is_begined)
        {
            if (_M_current_framebuffer)
            {
                _M_current_framebuffer->unbind(this);
            }

            _M_buffer.end();

            API->_M_graphics_queue.submit(_M_submit_info, _M_fence);
            VulkanCommandBufferState::reset();
            _M_is_begined = false;
        }
        return *this;
    }

    ThreadedCommandBuffer::~ThreadedCommandBuffer()
    {
        vulkan_device()->freeCommandBuffers(*vulkan_command_pool(), _M_buffer);
    }


    CommandBufferThread::CommandBufferThread()
    {
        _M_thread = new std::thread();
    }

    CommandBufferThread::~CommandBufferThread()
    {
        delete _M_thread;
    }

    VulkanAsyncCommandBuffer::VulkanAsyncCommandBuffer()
    {
        _M_secondary_command_buffers.resize(max_thread_count);

        for (auto& buffer : _M_secondary_command_buffers)
        {
            buffer = new ThreadedCommandBuffer(true);
        }

        _M_callback = recreate_swapchain_callback;
        _M_threads.resize(max_thread_count);
    }

    ThreadedCommandBuffer* VulkanAsyncCommandBuffer::get_threaded_command_buffer(bool force_base_framebuffer)
    {
        if (force_base_framebuffer || !_M_async_enabled)
        {
            return this;
        }

        ThreadedCommandBuffer* result = _M_secondary_command_buffers[_M_buffer_index];
        if (!result->_M_is_begined)
            result->begin();
        return result;
    }

    vk::CommandBuffer* VulkanAsyncCommandBuffer::get(bool force_base_framebuffer)
    {
        return &get_threaded_command_buffer(force_base_framebuffer)->_M_buffer;
    }

    VulkanAsyncCommandBuffer& VulkanAsyncCommandBuffer::begin_render()
    {
        begin();
        return *this;
    }


    static void async_call_end(ThreadedCommandBuffer* command_buffer)
    {
        command_buffer->end();
    }

    VulkanAsyncCommandBuffer& VulkanAsyncCommandBuffer::end_render()
    {

        // Execute async command buffers
        Index index = 0;
        for (ThreadedCommandBuffer* async_command_buffer : _M_secondary_command_buffers)
        {
            if (async_command_buffer->_M_is_begined)
            {
                _M_threads[index]._M_is_active      = true;
                _M_threads[index]._M_command_buffer = async_command_buffer;
                (*_M_threads[index]._M_thread)      = std::thread(async_call_end, async_command_buffer);
                ++index;
            }
        }

        // Wait end of executing async command buffers
        for (CommandBufferThread& thread : _M_threads)
        {
            if (!thread._M_is_active)
                break;

            thread._M_is_active = false;
            thread._M_thread->join();
            while (vk::Result::eTimeout ==
                   API->_M_device.waitForFences(thread._M_command_buffer->_M_fence, VK_TRUE, UINT32_MAX))
                ;
        }

        end();


        uint32_t swapchain_index = API->swapchain_image_index().value;

        vk::PresentInfoKHR present_info(_M_finished_semaphore, API->_M_swap_chain->_M_swap_chain, swapchain_index);
        vk::Result result = API->_M_present_queue.presentKHR(present_info);

        switch (result)
        {
            case vk::Result::eSuccess:
                break;

            case vk::Result::eErrorOutOfDateKHR:
            case vk::Result::eSuboptimalKHR:
                API->_M_need_recreate_swap_chain = true;
                break;

            default:
                assert(false);
        }

        if (API->_M_need_recreate_swap_chain)
            API->recreate_swap_chain();

        API->_M_current_command_buffer  = nullptr;
        API->_M_need_update_image_index = true;
        _M_buffer_index                 = 0;

        return *this;
    }

    VulkanAsyncCommandBuffer& VulkanAsyncCommandBuffer::next_render_thread()
    {
        _M_buffer_index = (_M_buffer_index + 1) % max_thread_count;
        return *this;
    }

    VulkanAsyncCommandBuffer::~VulkanAsyncCommandBuffer()
    {
        for (auto& buffer : _M_secondary_command_buffers)
        {
            delete buffer;
        }
        _M_secondary_command_buffers.clear();
    }

}// namespace Engine
