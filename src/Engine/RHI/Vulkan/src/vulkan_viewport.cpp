#include <Graphics/render_target.hpp>
#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{

    VulkanViewport::SyncObject::SyncObject()
    {
        m_image_present   = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
        m_render_finished = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
        m_fence           = API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    VulkanViewport::SyncObject::~SyncObject()
    {
        DESTROY_CALL(destroySemaphore, m_image_present);
        DESTROY_CALL(destroyFence, m_fence);
        DESTROY_CALL(destroySemaphore, m_render_finished);
    }

    void VulkanViewport::init()
    {
        m_command_buffers = API->m_device.allocateCommandBuffers(
                vk::CommandBufferAllocateInfo(API->m_command_pool, vk::CommandBufferLevel::ePrimary, API->m_framebuffers_count));

        m_sync_objects.resize(API->m_framebuffers_count);
    }

    void VulkanViewport::reinit()
    {
        m_sync_objects.clear();
        m_sync_objects.resize(API->m_framebuffers_count);
    }

    void VulkanViewport::begin_render()
    {
        SyncObject& sync = m_sync_objects[API->m_current_buffer];

        while (vk::Result::eTimeout == API->m_device.waitForFences(sync.m_fence, VK_TRUE, UINT64_MAX))
        {
        }

        API->m_device.resetFences(sync.m_fence);

        API->current_command_buffer().reset();
        API->current_command_buffer().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    }

    void VulkanViewport::end_render()
    {
        if (API->m_state->m_render_target)
        {
            API->m_state->m_render_target->unbind();
        }

        API->current_command_buffer().end();

        SyncObject& sync = m_sync_objects[API->m_current_buffer];

        static const vk::PipelineStageFlags wait_flags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submit_info(sync.m_image_present, wait_flags, API->current_command_buffer(), sync.m_render_finished);

        API->m_graphics_queue.submit(submit_info, sync.m_fence);
    }

    void VulkanViewport::on_resize(const Size2D& new_size)
    {}

    Identifier VulkanViewport::internal_type()
    {
        return VULKAN_VIEWPORT_ID;
    }

    bool VulkanViewport::vsync()
    {
        return false;
    }

    void VulkanViewport::vsync(bool flag)
    {}

    RHI_RenderTarget* VulkanViewport::render_target()
    {
        return m_render_target;
    }

    void VulkanViewport::destroy_image_views()
    {
        for (auto& view : m_image_views)
        {
            API->m_device.destroyImageView(view);
        }

        m_image_views.clear();
    }

    void VulkanViewport::before_begin_render()
    {
        API->m_state->reset();
        API->m_state->m_current_viewport = this;
    }

    void VulkanViewport::after_end_render()
    {
        API->m_state->m_current_viewport = nullptr;
    }


    VulkanViewport::~VulkanViewport()
    {
        API->m_device.freeCommandBuffers(API->m_command_pool, m_command_buffers);
        m_sync_objects.clear();
    }

    vk::CommandBuffer& VulkanAPI::current_command_buffer()
    {
        return m_state->m_current_viewport->m_command_buffers[API->m_current_buffer];
    }


    // Render Target Viewport

    VulkanViewport* VulkanRenderTargetViewport::init(RenderTarget* render_target)
    {
        VulkanViewport::init();
        m_render_target = render_target->rhi_object<VulkanRenderTarget>();

        return this;
    }

    void VulkanRenderTargetViewport::begin_render()
    {
        before_begin_render();
        m_buffer_index = (m_buffer_index + 1) % API->m_framebuffers_count;
    }

    void VulkanRenderTargetViewport::end_render()
    {
        VulkanViewport::end_render();
        after_end_render();
    }

    // Window Viewport

    VulkanViewport* VulkanWindowViewport::init(WindowInterface* window, bool vsync, bool need_initialize)
    {
        m_window  = window;
        m_surface = need_initialize ? API->m_surface : API->create_surface(window);

        VulkanViewport::init();
        m_present_mode = API->present_mode_of(vsync);
        create_swapchain();
        if (need_initialize)
        {
            API->create_render_pass(static_cast<vk::Format>(m_swapchain->image_format));
        }
        create_main_render_target();
        return this;
    }

    void VulkanWindowViewport::on_resize(const Size2D& new_size)
    {
        auto size = API->surface_size(m_surface);
        if (size.width != static_cast<std::uint32_t>(m_swapchain->extent.width) ||
            size.height != static_cast<uint32_t>(m_swapchain->extent.height))
        {
            m_need_recreate_swap_chain = true;
            reinterpret_cast<VulkanWindowRenderTarget*>(m_render_target)->frame()->size(size.width, size.height);
        }
    }

    bool VulkanWindowViewport::vsync()
    {
        return API->vsync_from_present_mode(m_present_mode);
    }

    void VulkanWindowViewport::vsync(bool flag)
    {
        m_present_mode             = API->present_mode_of(flag);
        m_need_recreate_swap_chain = true;
    }

    void VulkanWindowViewport::create_swapchain()
    {
        // Creating swapchain
        vulkan_info_log("Vulkan API", "Creating new swapchain");
        vkb::SwapchainBuilder swapchain_builder(API->m_bootstrap_device, m_surface);

        if (m_swapchain)
        {
            swapchain_builder.set_old_swapchain(m_swapchain->swapchain);
        }

        swapchain_builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(m_present_mode));

        size_t images_count = API->m_framebuffers_count;
        swapchain_builder.set_desired_min_image_count(images_count).set_required_min_image_count(images_count);

        swapchain_builder.add_image_usage_flags(
                static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst));
#if PLATFORM_ANDROID
        swapchain_builder.set_pre_transform_flags(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
#endif
        VkSurfaceFormatKHR f;
        f.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        f.format     = VK_FORMAT_B8G8R8A8_UNORM;
        swapchain_builder.set_desired_format(f);
 
        auto swap_ret = swapchain_builder.build();

        if (!swap_ret)
        {
            throw std::runtime_error(swap_ret.error().message());
        }

        if (m_swapchain)
        {
            destroy_swapchain(false);
        }

        if (!m_swapchain)
            m_swapchain = new vkb::Swapchain();

        (*m_swapchain) = swap_ret.value();

        auto images_result = m_swapchain->get_images();
        if (!images_result.has_value())
            throw EngineException(images_result.error().message());
        m_images = std::move(images_result.value());

        auto image_views_result = m_swapchain->get_image_views();
        if (!image_views_result.has_value())
            throw EngineException(image_views_result.error().message());
        m_image_views = std::move(image_views_result.value());
    }

    void VulkanWindowViewport::recreate_swapchain()
    {
        if (m_need_recreate_swap_chain)
        {
            m_need_recreate_swap_chain              = false;
            VulkanWindowRenderTarget* render_target = reinterpret_cast<VulkanWindowRenderTarget*>(m_render_target);
            API->wait_idle();

            render_target->destroy();
            create_swapchain();
            create_main_render_target();

            VulkanViewport::reinit();
        }
    }

    void VulkanWindowViewport::create_main_render_target()
    {
        if (!m_render_target)
            m_render_target = new VulkanWindowRenderTarget();

        VulkanWindowRenderTarget* render_target = reinterpret_cast<VulkanWindowRenderTarget*>(m_render_target);
        render_target->resize_count(m_swapchain->image_count);
        render_target->init(this);
    }

    void VulkanWindowViewport::destroy_swapchain(bool fully)
    {
        vkb::destroy_swapchain(*m_swapchain);
        destroy_image_views();

        if (fully)
        {
            delete m_swapchain;
            m_swapchain = nullptr;
        }
    }

    vk::ResultValue<uint32_t> VulkanWindowViewport::swapchain_image_index()
    {
        SyncObject& sync = m_sync_objects[API->m_current_buffer];
        try
        {
            return API->m_device.acquireNextImageKHR(m_swapchain->swapchain, UINT64_MAX, sync.m_image_present, nullptr);
        }
        catch (const std::exception& e)
        {
            return vk::ResultValue<uint32_t>(vk::Result::eErrorOutOfDateKHR, -1);
        }
    }

    VulkanWindowViewport::~VulkanWindowViewport()
    {
        delete m_render_target;
        destroy_swapchain(true);
        vk::Instance(API->m_instance.instance).destroySurfaceKHR(m_surface);
        m_window->destroy_api_context();
    }


    void VulkanWindowViewport::begin_render()
    {
        before_begin_render();
        recreate_swapchain();

        VulkanViewport::begin_render();

        auto current_buffer_index = swapchain_image_index();

        if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
        {
            m_need_recreate_swap_chain = true;
            recreate_swapchain();
            return begin_render();
        }

        if (current_buffer_index.result != vk::Result::eSuccess && current_buffer_index.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_buffer_index = current_buffer_index.value;
    }

    static void transition_swapchain_image(vk::Image image)
    {
        vk::CommandBuffer execute_command_buffer = API->current_command_buffer();
        static vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        vk::ImageMemoryBarrier barrier(vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eNone, vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::ePresentSrcKHR, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image,
                                       range);
        execute_command_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eFragmentShader |
                                                       vk::PipelineStageFlagBits::eVertexShader |
                                                       vk::PipelineStageFlagBits::eComputeShader,
                                               vk::PipelineStageFlagBits::eTopOfPipe, {}, {}, {}, barrier);
    }

    void VulkanWindowViewport::end_render()
    {
        if (!API->m_state->m_is_image_rendered_to_swapchain)
        {
            for (VkImage image : m_images)
            {
                transition_swapchain_image(vk::Image(image));
            }
        }

        VulkanViewport::end_render();

        SyncObject& sync = m_sync_objects[API->m_current_buffer];

        vk::SwapchainKHR swapchain = m_swapchain->swapchain;
        vk::PresentInfoKHR present_info(sync.m_render_finished, swapchain, m_buffer_index);
        vk::Result result;

        try
        {
            result = API->m_present_queue.presentKHR(present_info);
        }
        catch (const std::exception& e)
        {
            result = vk::Result::eErrorOutOfDateKHR;
        }

        switch (result)
        {
            case vk::Result::eSuccess:
                break;

            case vk::Result::eSuboptimalKHR:
#if !SKIP_SUBOPTIMAL_KHR_ERROR
                break;
#endif
            case vk::Result::eErrorOutOfDateKHR:
                m_need_recreate_swap_chain = true;
                break;

            default:
                assert(false);
        }


        recreate_swapchain();
        after_end_render();
    }

    // Creating Viewports

    RHI_Viewport* VulkanAPI::create_viewport(WindowInterface* interface, bool vsync)
    {
        bool need_initialize = m_instance == nullptr;
        if (need_initialize)
        {
            initialize(interface);
        }

        VulkanWindowViewport* viewport = new VulkanWindowViewport();
        viewport->init(interface, vsync, need_initialize);
        return viewport;
    }

    RHI_Viewport* VulkanAPI::create_viewport(RenderTarget* render_target)
    {
        VulkanRenderTargetViewport* viewport = new VulkanRenderTargetViewport();
        viewport->init(render_target);
        return viewport;
    }
}// namespace Engine
