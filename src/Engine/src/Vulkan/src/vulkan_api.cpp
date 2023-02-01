#include <Core/benchmark.hpp>
#include <VkBootstrap.h>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vulkan_api.hpp>
#include <vulkan_export.hpp>
#include <vulkan_object.hpp>
#include <vulkan_shader.hpp>


namespace Engine
{
    std::vector<const char*> VulkanAPI::device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const std::vector<const char*> validation_layers = {
#if ENABLE_VALIDATION_LAYERS
            "VK_LAYER_KHRONOS_validation"
#endif
    };

    VulkanAPI* VulkanAPI::_M_vulkan = nullptr;

    API_EXPORT GraphicApiInterface::ApiInterface* load_api()
    {
        if (VulkanAPI::_M_vulkan == nullptr)
            VulkanAPI::_M_vulkan = new VulkanAPI();
        return VulkanAPI::_M_vulkan;
    }

    VulkanObject::~VulkanObject()
    {}

    VulkanAPI::VulkanAPI() : _M_current_buffer_index(vk::Result::eSuccess, 0)
    {}


    VulkanAPI::~VulkanAPI()
    {}

    VulkanAPI& VulkanAPI::destroy_window()
    {
        _M_device.waitIdle();

        destroy_framebuffers();
        delete _M_swap_chain;

        _M_device.destroyRenderPass(_M_render_pass);

        for (auto& ell : _M_image_available_semaphores) _M_device.destroySemaphore(ell);
        for (auto& ell : _M_render_finished_semaphores) _M_device.destroySemaphore(ell);
        for (auto& ell : _M_in_flight_fences) _M_device.destroyFence(ell);


        _M_device.destroyCommandPool(_M_command_pool);

        _M_device.destroy();

        vk::Instance(_M_instance.instance).destroySurfaceKHR(_M_surface);
        vkb::destroy_instance(_M_instance);
        return *this;
    }


    static void shader_test()
    {
        auto read_file = [](const std::string& file) -> FileBuffer {
            std::ifstream file_stream(file, std::ios::ate | std::ios::binary);

            size_t file_size = (size_t) file_stream.tellg();
            FileBuffer buffer(file_size);

            file_stream.seekg(0);
            file_stream.read((char*) buffer.data(), static_cast<std::streamsize>(file_size));
            file_stream.close();
            return buffer;
        };

        std::string fragment = "/home/programier/Projects/Shaders/frag.spv";
        std::string vertex = "/home/programier/Projects/Shaders/vert.spv";

        ShaderParams params;
        params.name = "main";
        params.binaries.vertex = read_file(vertex);
        params.binaries.fragment = read_file(fragment);
        VulkanShader shader;
        try
        {
            shader.init(params);
        }
        catch (...)
        {
            std::clog << "SHADER TEST FAILED!" << std::endl;
        }
    }

    void* VulkanAPI::init_window(SDL_Window* window)
    {
        _M_window = window;
        auto funcs = {&VulkanAPI::init,
                      &VulkanAPI::create_swap_chain,
                      &VulkanAPI::create_render_pass,
                      &VulkanAPI::create_framebuffers,
                      &VulkanAPI::create_command_buffer,
                      &VulkanAPI::create_semaphores};

        try
        {
            for (auto func : funcs) (this->*func)();
        }
        catch (const std::exception& e)
        {
            std::clog << e.what() << std::endl;
            return nullptr;
        }

        shader_test();
        return static_cast<VkSurfaceKHR>(_M_surface);
    }


    ///////////////////////////////// INITIALIZATION /////////////////////////////////

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static std::vector<const char*> get_required_extensions(SDL_Window* window)
    {
        uint32_t count = 0;

        SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
        std::vector<const char*> extensions(count);
        SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());

        return extensions;
    }

    void VulkanAPI::init()
    {
        vkb::InstanceBuilder instance_builder;
        for (auto& extension : get_required_extensions(_M_window))
        {
            instance_builder.enable_extension(extension);
        }

        auto instance_ret = instance_builder.set_debug_callback(debugCallback).request_validation_layers().build();
        if (!instance_ret)
        {
            throw std::runtime_error(instance_ret.error().message());
        }

        _M_instance = instance_ret.value();

        create_surface();

        vkb::PhysicalDeviceSelector phys_device_selector(instance_ret.value());

        auto phys_device_ret = phys_device_selector.set_surface(static_cast<VkSurfaceKHR>(_M_surface)).select();
        if (!phys_device_ret)
        {
            throw std::runtime_error(phys_device_ret.error().message());
        }

        _M_physical_device = vk::PhysicalDevice(phys_device_ret.value().physical_device);

        vkb::DeviceBuilder device_builder(phys_device_ret.value());

        auto device_ret = device_builder.build();
        if (!device_ret)
        {
            throw std::runtime_error(device_ret.error().message());
        }

        _M_bootstrap_device = device_ret.value();
        _M_device = vk::Device(device_ret.value().device);


        auto index_1 = _M_bootstrap_device.get_queue_index(vkb::QueueType::graphics);
        auto index_2 = _M_bootstrap_device.get_queue_index(vkb::QueueType::present);
        auto graphics_queue = _M_bootstrap_device.get_queue(vkb::QueueType::graphics);
        auto present_queue = _M_bootstrap_device.get_queue(vkb::QueueType::present);

        if (!index_1 || !index_2 || !graphics_queue || !present_queue)
        {
            throw std::runtime_error("Failed to init queues");
        }

        _M_graphics_and_present_index.graphics_family = index_1.value();
        _M_graphics_and_present_index.present_family = index_2.value();

        _M_graphics_queue = vk::Queue(graphics_queue.value());
        _M_present_queue = vk::Queue(present_queue.value());
    }


    void VulkanAPI::create_surface()
    {
        VkSurfaceKHR _surface;
        SDL_Vulkan_CreateSurface(_M_window, static_cast<VkInstance>(_M_instance), &_surface);
        _M_surface = vk::SurfaceKHR(_surface);
    }

    void VulkanAPI::destroy_framebuffers()
    {
        for (auto& framebuffer : _M_swap_chain_framebuffers)
        {
            delete framebuffer;
        }

        _M_swap_chain_framebuffers.clear();
    }

    void VulkanAPI::recreate_swap_chain()
    {
        _M_device.waitIdle();

        _M_device.destroyCommandPool(_M_command_pool);

        destroy_framebuffers();
        create_swap_chain();
        create_framebuffers();
        create_command_buffer();
        _M_need_recreate_swap_chain = false;
    }


    void VulkanAPI::create_swap_chain()
    {
        SDL_Vulkan_GetDrawableSize(_M_window, &window_data.width, &window_data.height);
        _M_swap_chain = new SwapChain();
    }

    void VulkanAPI::create_render_pass()
    {
        vk::Format color_format = _M_swap_chain->_M_format;
        std::array<vk::AttachmentDescription, 1> attachment_descriptions;
        attachment_descriptions[0] = vk::AttachmentDescription(
                vk::AttachmentDescriptionFlags(), color_format, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);


        vk::AttachmentReference color_reference(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, {},
                                       color_reference);

        vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                         vk::PipelineStageFlagBits::eColorAttachmentOutput, {},
                                         vk::AccessFlagBits::eColorAttachmentWrite |
                                                 vk::AccessFlagBits::eColorAttachmentRead);

        _M_render_pass = _M_device.createRenderPass(
                vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachment_descriptions, subpass, dependency));
    }


    void VulkanAPI::create_framebuffers()
    {
        _M_swap_chain_framebuffers.resize(_M_swap_chain->_M_images.size());
        std::array<vk::ImageView, 1> attachments;


        vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlags(), _M_render_pass, attachments,
                                                          _M_swap_chain->_M_extent.width,
                                                          _M_swap_chain->_M_extent.height, 1);
        std::size_t index = 0;
        for (auto const& image_view : _M_swap_chain->_M_image_views)
        {
            attachments[0] = image_view;
            _M_swap_chain_framebuffers[index] = new VulkanFramebuffer();
            _M_swap_chain_framebuffers[index]->_M_framebuffer = _M_device.createFramebuffer(framebuffer_create_info);
            _M_swap_chain_framebuffers[index]->_M_render_pass = _M_render_pass;

            _M_swap_chain_framebuffers[index]->init_render_pass_info();
            index++;
        }
    }

    void VulkanAPI::create_command_buffer()
    {
        _M_command_pool = _M_device.createCommandPool(
                vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                          _M_graphics_and_present_index.graphics_family.value()));

        _M_command_buffers = _M_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(
                _M_command_pool, vk::CommandBufferLevel::ePrimary, MAIN_FRAMEBUFFERS_COUNT));
    }

    void VulkanAPI::create_semaphores()
    {

        _M_image_available_semaphores.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_render_finished_semaphores.resize(MAIN_FRAMEBUFFERS_COUNT);
        _M_in_flight_fences.resize(MAIN_FRAMEBUFFERS_COUNT);

        for (size_t i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++)
        {
            _M_image_available_semaphores[i] = _M_device.createSemaphore(vk::SemaphoreCreateInfo());
            _M_render_finished_semaphores[i] = _M_device.createSemaphore(vk::SemaphoreCreateInfo());
            _M_in_flight_fences[i] = _M_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        }
    }

    VulkanAPI& VulkanAPI::on_window_size_changed()
    {
        _M_need_recreate_swap_chain = true;
        return *this;
    }

    VulkanAPI& VulkanAPI::swap_buffer(SDL_Window* window)
    {
        _M_current_frame = (_M_current_frame + 1) % MAIN_FRAMEBUFFERS_COUNT;
        return *this;
    }

    VulkanAPI& VulkanAPI::begin_render()
    {
        while (vk::Result::eTimeout ==
               _M_device.waitForFences({_M_in_flight_fences[_M_current_frame]}, VK_TRUE, UINT64_MAX))
        {}

        _M_current_buffer_index = _M_device.acquireNextImageKHR(
                _M_swap_chain->_M_swap_chain, UINT64_MAX, _M_image_available_semaphores[_M_current_frame], nullptr);

        if (_M_current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
        {
            recreate_swap_chain();
            return begin_render();
        }
        else if (_M_current_buffer_index.result != vk::Result::eSuccess &&
                 _M_current_buffer_index.result != vk::Result::eSuboptimalKHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        //        if (_M_image_in_flight[_M_current_buffer_index.value])
        //        {
        //            (void) _M_device.waitForFences(_M_image_in_flight[_M_current_buffer_index.value], VK_TRUE, UINT64_MAX);
        //        }

        //        _M_image_in_flight[_M_current_buffer_index.value] = _M_in_flight_fences[_M_current_frame];

        _M_device.resetFences({_M_in_flight_fences[_M_current_frame]});
        _M_current_command_buffer = &_M_command_buffers[_M_current_frame];
        _M_current_command_buffer->reset();

        _M_current_command_buffer->begin(vk::CommandBufferBeginInfo());

        return *this;
    }

    VulkanAPI& VulkanAPI::end_render()
    {

        // #########################################
        _M_current_command_buffer->end();

        vk::PipelineStageFlags wait_destination_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        std::array<vk::Semaphore, 1> wait_semaphores = {_M_image_available_semaphores[_M_current_frame]};
        std::array<vk::Semaphore, 1> signal_semaphores = {_M_render_finished_semaphores[_M_current_frame]};

        vk::SubmitInfo submit_info(wait_semaphores, wait_destination_stage_mask, *_M_current_command_buffer,
                                   signal_semaphores);

        _M_graphics_queue.submit(submit_info, _M_in_flight_fences[_M_current_frame]);

        vk::PresentInfoKHR present_info(signal_semaphores, _M_swap_chain->_M_swap_chain, _M_current_buffer_index.value);

        vk::Result result = _M_present_queue.presentKHR(present_info);


        switch (result)
        {
            case vk::Result::eSuccess:
                break;

            case vk::Result::eErrorOutOfDateKHR:
            case vk::Result::eSuboptimalKHR:
                recreate_swap_chain();
                break;
            default:
                assert(false);
        }

        if (_M_need_recreate_swap_chain)
        {
            recreate_swap_chain();
        }

        _M_current_framebuffer = nullptr;
        return *this;
    }

    VulkanAPI& VulkanAPI::begin_render_pass()
    {
        _M_current_command_buffer->beginRenderPass(_M_current_framebuffer->_M_render_pass_info,
                                                   vk::SubpassContents::eInline);
        return *this;
    }

    VulkanAPI& VulkanAPI::end_render_pass()
    {
        _M_current_command_buffer->endRenderPass();
        return *this;
    }

    VulkanAPI& VulkanAPI::framebuffer_viewport(const Point2D& point, const Size2D& size)
    {
        window_data.view_port.x = static_cast<int_t>(point.x);
        window_data.view_port.y = static_cast<int_t>(point.y);
        window_data.view_port.width = static_cast<int_t>(size.x);
        window_data.view_port.height = static_cast<int_t>(size.y);

        if (_M_current_framebuffer)
        {
            _M_current_framebuffer->update_viewport();
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::bind_framebuffer(const ObjID& ID)
    {
        if (ID == 0)
        {
            _M_swap_chain_framebuffers[_M_current_buffer_index.value]->bind();
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::clear_color(const Color& color)
    {
        _M_clear_values[0].setColor(vk::ClearColorValue(std::array<float, 4>{color.x, color.y, color.z, color.a}));
        return *this;
    }

    VulkanAPI& VulkanAPI::clear_frame_buffer(const ObjID& ID, BufferType type)
    {
        _M_current_framebuffer->clear_color(type);
        return *this;
    }

    VulkanAPI& VulkanAPI::swap_interval(int_t interval)
    {
        static const std::unordered_map<int_t, vk::PresentModeKHR> modes = {{-1, vk::PresentModeKHR::eFifoRelaxed},
                                                                            {0, vk::PresentModeKHR::eMailbox},
                                                                            {1, vk::PresentModeKHR::eFifo}};
        int_t index = interval != 0 ? interval / glm::abs(interval) : interval;
        interval = glm::abs(interval);
        _M_swap_chain_mode = modes.at(index);
        _M_need_recreate_swap_chain = true;
        return *this;
    }

}// namespace Engine
