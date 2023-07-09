#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>


namespace Engine
{
    extern vk::Device* vulkan_device();
    extern vk::CommandPool* vulkan_command_pool();

    static const std::uint32_t max_thread_count = std::thread::hardware_concurrency();


    static FORCE_INLINE void recreate_swapchain_callback()
    {
        API->_M_need_recreate_swap_chain = true;
        API->_M_need_update_image_index  = true;
        API->recreate_swap_chain();
    }

    VulkanSyncObject::VulkanSyncObject()
    {
        _M_available_semaphore = vulkan_device()->createSemaphore(vk::SemaphoreCreateInfo());
        _M_finished_semaphore  = vulkan_device()->createSemaphore(vk::SemaphoreCreateInfo());
        _M_fence               = vulkan_device()->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    VulkanSyncObject& VulkanSyncObject::wait()
    {
        while (vk::Result::eTimeout == API->_M_device.waitForFences(_M_fence, VK_TRUE, UINT64_MAX))
        {}

        API->_M_device.resetFences(_M_fence);
        return *this;
    }

    VulkanSyncObject& VulkanSyncObject::submit(vk::CommandBuffer& buffer)
    {
        static const vk::PipelineStageFlags wait_flags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

        API->_M_graphics_queue.submit(vk::SubmitInfo(_M_available_semaphore, wait_flags, buffer, _M_finished_semaphore),
                                      _M_fence);
        return *this;
    }

    VulkanSyncObject::~VulkanSyncObject()
    {
        DESTROY_CALL(destroySemaphore, _M_available_semaphore);
        DESTROY_CALL(destroySemaphore, _M_finished_semaphore);
        DESTROY_CALL(destroyFence, _M_fence);
    }

    VulkanCommandBufferState& VulkanCommandBufferState::reset()
    {
        _M_current_framebuffer   = nullptr;
        _M_current_shader        = nullptr;
        _M_current_vertex_buffer = nullptr;
        _M_current_index_buffer  = nullptr;
        return *this;
    }

    VulkanCommandBufferThreadedEndCommand::VulkanCommandBufferThreadedEndCommand(vk::CommandBuffer* buffer,
                                                                                 VulkanSyncObject* sync)
        : _M_buffer(buffer), _M_sync_object(sync)
    {
        _M_thread = new std::thread(thread_loop, this);
    }

    void VulkanCommandBufferThreadedEndCommand::thread_loop(VulkanCommandBufferThreadedEndCommand* command)
    {
        while (!command->_M_destruct)
        {
            {
                std::unique_lock lock(command->_M_mutex);
                command->_M_cv.wait(lock);
                if (command->_M_destruct)
                {
                    return;
                }
            }

            command->execute();
        }
    }

    void VulkanCommandBufferThreadedEndCommand::execute()
    {
        bool is_threaded = std::this_thread::get_id() == _M_thread->get_id();

        if (is_threaded)
        {
            _M_mutex.lock();
            API->_M_active_threads.fetch_add(1);
        }

        _M_sync_object->submit(*_M_buffer);
        vk::PresentInfoKHR present_info(_M_sync_object->_M_finished_semaphore, API->_M_swap_chain->_M_swap_chain,
                                        _M_swapchain_index);
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

        if (is_threaded)
        {
            if (API->_M_need_recreate_swap_chain)
            {
                API->_M_enabled_threaded_end_command.store(false);
            }

            API->_M_active_threads.fetch_sub(1);
            _M_mutex.unlock();
        }
        else if (API->_M_need_recreate_swap_chain)
        {
            API->recreate_swap_chain();
        }
    }

    void VulkanCommandBufferThreadedEndCommand::notify()
    {
        _M_cv.notify_all();
    }

    void VulkanCommandBufferThreadedEndCommand::wait()
    {
        std::unique_lock lock(_M_mutex);
    }

    VulkanCommandBufferThreadedEndCommand::~VulkanCommandBufferThreadedEndCommand()
    {
        {
            std::unique_lock lock(_M_mutex);
            _M_destruct = true;
        }
        wait();
        _M_cv.notify_all();
        _M_thread->join();
        delete _M_thread;
    }


    VulkanCommandBuffer::VulkanCommandBuffer()
    {
        _M_sync_objects.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_command_buffer_state.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_command_buffers.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_end_commands.resize(MAIN_FRAMEBUFFERS_COUNT);

        {
            vk::CommandBufferAllocateInfo info(API->_M_command_pool, vk::CommandBufferLevel::ePrimary,
                                               MAIN_FRAMEBUFFERS_COUNT);

            auto buffers = API->_M_device.allocateCommandBuffers(info);
            for (int i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
            {
                _M_command_buffers[i] = std::move(buffers[i]);
                _M_end_commands[i] =
                        new VulkanCommandBufferThreadedEndCommand(&_M_command_buffers[i], &_M_sync_objects[i]);
            }
        }
    }

    VulkanCommandBuffer& VulkanCommandBuffer::begin()
    {
        sync_object().wait();

        vk::ResultValue<uint32_t> current_buffer_index(vk::Result::eSuccess, 0);

        do
        {
            current_buffer_index = API->swapchain_image_index();
            if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
            {
                recreate_swapchain_callback();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        } while (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR);

        if (current_buffer_index.result != vk::Result::eSuccess &&
            current_buffer_index.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vk::CommandBuffer& buffer = get();
        buffer.reset();
        buffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        return *this;
    }


    VulkanCommandBuffer& VulkanCommandBuffer::end()
    {
        VulkanCommandBufferState& buffer_state = state();

        if (buffer_state._M_current_framebuffer)
        {
            buffer_state._M_current_framebuffer->unbind();
        }

        auto current_end_command                = end_command();
        current_end_command->_M_swapchain_index = API->swapchain_image_index().value;

        vk::CommandBuffer& buffer = get();
        buffer.end();
        state().reset();
        API->_M_need_update_image_index = true;


        for (VulkanCommandBufferThreadedEndCommand* command : _M_end_commands)
        {
            command->wait();
        }

#if USE_THREADED_END_COMMAND
        if (API->_M_enabled_threaded_end_command.load())
        {
            current_end_command->notify();
        }
        else
#endif
        {
            current_end_command->execute();
        }


        return *this;
    }

    vk::CommandBuffer& VulkanCommandBuffer::get()
    {
        return _M_command_buffers[API->_M_current_frame];
    }

    VulkanCommandBufferState& VulkanCommandBuffer::state()
    {
        return _M_command_buffer_state[API->_M_current_frame];
    }

    VulkanSyncObject& VulkanCommandBuffer::sync_object()
    {
        return _M_sync_objects[API->_M_current_frame];
    }

    VulkanCommandBufferThreadedEndCommand* VulkanCommandBuffer::end_command()
    {
        return _M_end_commands[API->_M_current_frame];
    }

    VulkanCommandBuffer::operator vk::CommandBuffer&()
    {
        return get();
    }

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        for (auto& buffer : _M_command_buffers)
        {
            API->_M_device.freeCommandBuffers(API->_M_command_pool, buffer);
        }

        for (VulkanCommandBufferThreadedEndCommand* command : _M_end_commands)
        {
            delete command;
        }
    }

}// namespace Engine
