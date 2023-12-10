#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{

    VulkanViewport::SyncObject::SyncObject()
    {
        _M_image_present   = API->_M_device.createSemaphore(vk::SemaphoreCreateInfo());
        _M_render_finished = API->_M_device.createSemaphore(vk::SemaphoreCreateInfo());
        _M_fence           = API->_M_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
    }

    VulkanViewport::SyncObject::~SyncObject()
    {
        DESTROY_CALL(destroySemaphore, _M_image_present);
        DESTROY_CALL(destroyFence, _M_fence);
        DESTROY_CALL(destroySemaphore, _M_render_finished);
    }

    void VulkanViewport::init()
    {
        _M_command_buffers = API->_M_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
                API->_M_command_pool, vk::CommandBufferLevel::ePrimary, API->_M_framebuffers_count));

        _M_sync_objects.resize(API->_M_framebuffers_count);
    }


    VulkanViewport* tmp = nullptr;
    void VulkanViewport::create_swapchain()
    {
        // Creating swapchain
        vulkan_info_log("Vulkan API", "Creating new swapchain");
        vkb::SwapchainBuilder swapchain_builder(API->_M_bootstrap_device, _M_surface);

        if (_M_swapchain)
        {
            swapchain_builder.set_old_swapchain(_M_swapchain->swapchain);
        }

        swapchain_builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(_M_present_mode));

        size_t images_count = API->_M_framebuffers_count;
        swapchain_builder.set_desired_min_image_count(images_count).set_required_min_image_count(images_count);

        swapchain_builder.add_image_usage_flags(static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferSrc |
                                                                               vk::ImageUsageFlagBits::eTransferDst));
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

        if (_M_swapchain)
        {
            destroy_swapchain(false);
        }

        if (!_M_swapchain)
            _M_swapchain = new vkb::Swapchain();

        (*_M_swapchain) = swap_ret.value();


        auto image_result = _M_swapchain->get_images();
        if (!image_result.has_value())
            throw EngineException(image_result.error().message());
        _M_images = std::move(image_result.value());

        auto image_views_result = _M_swapchain->get_image_views();
        if (!image_views_result.has_value())
            throw EngineException(image_views_result.error().message());
        _M_image_views = std::move(image_views_result.value());

        tmp = this;
    }


    void VulkanViewport::create_main_render_target()
    {
        if (!_M_render_target)
            _M_render_target = new VulkanMainFrameBuffer();

        VulkanMainFrameBuffer* main_framebuffer = reinterpret_cast<VulkanMainFrameBuffer*>(_M_render_target);
        main_framebuffer->resize_count(_M_swapchain->image_count);
        main_framebuffer->init(this);
    }

    void VulkanViewport::recreate_swapchain()
    {
        if (_M_need_recreate_swap_chain)
        {
            _M_need_recreate_swap_chain             = false;
            VulkanMainFrameBuffer* main_framebuffer = reinterpret_cast<VulkanMainFrameBuffer*>(_M_render_target);
            API->wait_idle();
            API->_M_graphics_queue.waitIdle();
            API->_M_present_queue.waitIdle();

            main_framebuffer->destroy();
            create_swapchain();
            create_main_render_target();

            _M_sync_objects.clear();
            _M_sync_objects.resize(API->_M_framebuffers_count);
        }
    }

    VulkanViewport* VulkanViewport::init(WindowInterface* window, bool vsync, bool create_render_pass)
    {
        if (create_render_pass)
        {
            _M_surface = API->_M_surface;
        }
        else
        {
            _M_surface = API->create_surface(window);
        }


        init();
        _M_present_mode = API->present_mode_of(vsync);
        create_swapchain();
        if (create_render_pass)
        {
            API->create_render_pass(static_cast<vk::Format>(_M_swapchain->image_format));
        }
        create_main_render_target();


        return this;
    }

    vk::ResultValue<uint32_t> VulkanViewport::swapchain_image_index()
    {
        SyncObject& sync = _M_sync_objects[API->_M_current_buffer];
        return API->_M_device.acquireNextImageKHR(_M_swapchain->swapchain, UINT64_MAX, sync._M_image_present, nullptr);
    }

    VulkanViewport* VulkanViewport::init(RenderTarget* render_target)
    {
        init();
        return this;
    }

    void VulkanViewport::begin_render_window()
    {
        recreate_swapchain();

        auto current_buffer_index = swapchain_image_index();


        if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
        {
            _M_need_recreate_swap_chain = true;
            recreate_swapchain();
            return begin_render_window();
        }

        if (current_buffer_index.result != vk::Result::eSuccess &&
            current_buffer_index.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        _M_buffer_index = current_buffer_index.value;
    }

    void VulkanViewport::begin_render_render_target()
    {
        _M_buffer_index = (_M_buffer_index + 1) % API->_M_framebuffers_count;
    }

    void VulkanViewport::begin_render()
    {
        API->_M_current_viewport = this;
        _M_state.reset();

        if (_M_swapchain)
        {
            begin_render_window();
        }
        else
        {
            begin_render_render_target();
        }

        SyncObject& sync = _M_sync_objects[API->_M_current_buffer];

        while (vk::Result::eTimeout == API->_M_device.waitForFences(sync._M_fence, VK_TRUE, UINT64_MAX))
        {
        }

        API->_M_device.resetFences(sync._M_fence);

        API->current_command_buffer().reset();
        API->current_command_buffer().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    }

    void VulkanViewport::end_render()
    {
        if (_M_state._M_framebuffer)
        {
            _M_state._M_framebuffer->unbind();
        }

        API->current_command_buffer().end();


        SyncObject& sync = _M_sync_objects[API->_M_current_buffer];

        static const vk::PipelineStageFlags wait_flags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        vk::SubmitInfo submit_info(sync._M_image_present, wait_flags, API->current_command_buffer(),
                                   sync._M_render_finished);

        API->_M_graphics_queue.submit(submit_info, sync._M_fence);


        vk::SwapchainKHR swapchain = _M_swapchain->swapchain;

        vk::PresentInfoKHR present_info(sync._M_render_finished, swapchain, _M_buffer_index);
        vk::Result result;
        try
        {
            result = API->_M_present_queue.presentKHR(present_info);
        }
        catch (const std::exception& e)
        {
            result = vk::Result::eErrorOutOfDateKHR;
        }

        switch (result)
        {
            case vk::Result::eSuccess:
                break;

            case vk::Result::eErrorOutOfDateKHR:
#if !SKIP_SUBOPTIMAL_KHR_ERROR
            case vk::Result::eSuboptimalKHR:
#endif
                _M_need_recreate_swap_chain = true;
                break;

            default:
                assert(false);
        }


        recreate_swapchain();

        API->_M_current_viewport = nullptr;
    }

    void VulkanViewport::on_resize(const Size2D& new_size)
    {
        if (_M_swapchain)
        {
            auto size = API->surface_size(_M_surface);
            if (size.width != static_cast<std::uint32_t>(_M_swapchain->extent.width) ||
                size.height != static_cast<uint32_t>(_M_swapchain->extent.height))
            {
                _M_need_recreate_swap_chain = true;
                _M_render_target->size(size.width, size.height);
            }
        }
    }

    bool VulkanViewport::vsync()
    {
        return false;
    }

    void VulkanViewport::vsync(bool flag)
    {}

    RHI_RenderTarget* VulkanViewport::render_target()
    {
        return _M_render_target;
    }


    void VulkanViewport::destroy(bool fully)
    {
        API->wait_idle();
        if (fully)
            API->_M_device.freeCommandBuffers(API->_M_command_pool, _M_command_buffers);

        for (auto& view : _M_image_views)
        {
            API->_M_device.destroyImageView(view);
        }

        _M_image_views.clear();

        if (_M_swapchain)
        {
            destroy_swapchain(fully);

            if (fully)
            {
                vk::Instance(API->_M_instance.instance).destroySurfaceKHR(_M_surface);
            }
        }

        if (fully)
        {
            _M_sync_objects.clear();
        }
    }

    void VulkanViewport::destroy_swapchain(bool fully)
    {
        vkb::destroy_swapchain(*_M_swapchain);

        for (auto& view : _M_image_views)
        {
            API->_M_device.destroyImageView(view);
        }

        _M_image_views.clear();

        if (fully)
        {
            delete _M_swapchain;
            delete _M_render_target;
        }
    }

    VulkanViewport::~VulkanViewport()
    {
        destroy(true);
    }

    vk::CommandBuffer& VulkanAPI::current_command_buffer()
    {
        return _M_current_viewport->_M_command_buffers[API->_M_current_buffer];
    }

    struct VulkanState* VulkanAPI::state()
    {
        if (_M_current_viewport)
            return &_M_current_viewport->_M_state;
        return nullptr;
    }

    RHI_Viewport* VulkanAPI::create_viewport(WindowInterface* interface, bool vsync)
    {
        bool need_initialize = _M_instance == nullptr;
        if (need_initialize)
        {
            initialize(interface);
        }

        VulkanViewport* viewport = new VulkanViewport();
        viewport->init(interface, vsync, need_initialize);
        return viewport;
    }

    RHI_Viewport* VulkanAPI::create_viewport(RenderTarget* render_target)
    {
        VulkanViewport* viewport = new VulkanViewport();
        viewport->init(render_target);
        return viewport;
    }
}// namespace Engine
