#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
    extern vk::Device* vulkan_device();
    extern vk::CommandPool* vulkan_command_pool();

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


    VulkanCommandBuffer::VulkanCommandBuffer()
    {
        _M_sync_objects.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_command_buffers.resize(MAIN_FRAMEBUFFERS_COUNT);

        {
            vk::CommandBufferAllocateInfo info(API->_M_command_pool, vk::CommandBufferLevel::ePrimary,
                                               MAIN_FRAMEBUFFERS_COUNT);

            auto buffers = API->_M_device.allocateCommandBuffers(info);
            for (int i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
            {
                _M_command_buffers[i] = std::move(buffers[i]);
            }
        }
    }

    VulkanCommandBuffer& VulkanCommandBuffer::begin()
    {
        sync_object().wait();
        API->_M_need_update_image_index = true;
        auto current_buffer_index       = API->swapchain_image_index();


        if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
        {
            recreate_swapchain_callback();
            return *this;
        }


        API->_M_device.resetFences(sync_object()._M_fence);

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
        if (API->_M_state->_M_framebuffer)
        {
            API->_M_state->_M_framebuffer->unbind();
        }

        vk::CommandBuffer& buffer = get();
        buffer.end();

        VulkanSyncObject& current_sync_object = sync_object().submit(buffer);

        auto swapchain_index = API->swapchain_image_index().value;

        vk::PresentInfoKHR present_info(current_sync_object._M_finished_semaphore, API->_M_swap_chain->_M_swap_chain,
                                        swapchain_index);
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
        {
            API->recreate_swap_chain();
        }

        return *this;
    }

    vk::CommandBuffer& VulkanCommandBuffer::get()
    {
        return _M_command_buffers[API->_M_current_buffer];
    }

    VulkanSyncObject& VulkanCommandBuffer::sync_object()
    {
        return _M_sync_objects[API->_M_current_buffer];
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
    }

}// namespace Engine
