#include <Core/benchmark.hpp>
#include <VkBootstrap.h>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vulkan_api.hpp>
#include <vulkan_export.hpp>
#include <vulkan_mesh.hpp>
#include <vulkan_object.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_texture.hpp>


namespace Engine
{
    std::vector<const char*> VulkanAPI::device_extensions = {VK_KHR_MAINTENANCE1_EXTENSION_NAME};
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


        return static_cast<VkSurfaceKHR>(_M_surface);
    }


    ///////////////////////////////// INITIALIZATION /////////////////////////////////
#if ENABLE_VALIDATION_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData)
    {
        std::cerr << pCallbackData->pMessage << std::endl << std::endl;

        return VK_FALSE;
    }
#endif

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

#if ENABLE_VALIDATION_LAYERS
        instance_builder.set_debug_callback(debugCallback).request_validation_layers();
#endif

        auto instance_ret = instance_builder.build();

        if (!instance_ret)
        {
            throw std::runtime_error(instance_ret.error().message());
        }

        _M_instance = instance_ret.value();

        create_surface();

        vkb::PhysicalDeviceSelector phys_device_selector(instance_ret.value());
        vk::PhysicalDeviceFeatures features;
        features.samplerAnisotropy = VK_TRUE;
        phys_device_selector.set_required_features(static_cast<VkPhysicalDeviceFeatures>(features));

        phys_device_selector.add_required_extensions(device_extensions);

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
        wait_idle();

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
        size_t index = 0;
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

    static vk::Viewport inverse_viewport()
    {
        vk::Viewport view_port;
        view_port.x = API->window_data.view_port.x;
        view_port.y = API->window_data.view_port.height - view_port.x - API->window_data.view_port.y;
        view_port.width = API->window_data.view_port.width;
        view_port.height = -API->window_data.view_port.height;
        return view_port;
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

        _M_device.resetFences({_M_in_flight_fences[_M_current_frame]});
        _M_current_command_buffer = &_M_command_buffers[_M_current_frame];
        _M_current_command_buffer->reset();

        _M_current_command_buffer->begin(vk::CommandBufferBeginInfo());

        _M_current_command_buffer->setScissor(0, vk::Rect2D({0, 0}, _M_swap_chain->_M_extent));
        _M_current_command_buffer->setViewport(0, inverse_viewport());

        return *this;
    }

    VulkanAPI& VulkanAPI::end_render()
    {
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
        _M_current_command_buffer = nullptr;
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
        // _M_current_command_buffer->draw(3, 1, 0, 0);
        _M_current_command_buffer->endRenderPass();
        return *this;
    }

    uint32_t VulkanAPI::find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties mem_properties = _M_physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("VulkanAPI: Failed to find suitable memory type!");
    }

    VulkanAPI& VulkanAPI::create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                        vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
                                        vk::DeviceMemory& buffer_memory)
    {
        vk::BufferCreateInfo buffer_info({}, size, usage, vk::SharingMode::eExclusive);

        buffer = _M_device.createBuffer(buffer_info);


        vk::MemoryRequirements mem_requirements = _M_device.getBufferMemoryRequirements(buffer);

        vk::MemoryAllocateInfo alloc_info(mem_requirements.size,
                                          find_memory_type(mem_requirements.memoryTypeBits, properties));


        buffer_memory = _M_device.allocateMemory(alloc_info);
        _M_device.bindBufferMemory(buffer, buffer_memory, 0);
        return *this;
    }

    vk::CommandBuffer VulkanAPI::begin_single_time_command_buffer()
    {
        vk::CommandBufferAllocateInfo alloc_info(_M_command_pool, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer command_buffer = _M_device.allocateCommandBuffers(alloc_info).front();
        vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        command_buffer.begin(begin_info);
        return command_buffer;
    }

    VulkanAPI& VulkanAPI::end_single_time_command_buffer(const vk::CommandBuffer& command_buffer)
    {
        command_buffer.end();
        vk::SubmitInfo submit_info({}, {}, command_buffer);
        _M_graphics_queue.submit(submit_info, {});
        _M_graphics_queue.waitIdle();
        _M_device.freeCommandBuffers(_M_command_pool, command_buffer);
        return *this;
    }

    VulkanAPI& VulkanAPI::copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size,
                                      vk::DeviceSize src_offset, vk::DeviceSize dst_offset)
    {
        auto command_buffer = begin_single_time_command_buffer();
        vk::BufferCopy copy_region(src_offset, dst_offset, size);
        command_buffer.copyBuffer(src_buffer, dst_buffer, copy_region);
        return end_single_time_command_buffer(command_buffer);
    }

    VulkanAPI& VulkanAPI::framebuffer_viewport(const Point2D& point, const Size2D& size)
    {
        window_data.view_port.x = point.x;
        window_data.view_port.y = point.y;
        window_data.view_port.width = size.x;
        window_data.view_port.height = size.y;


        if (_M_current_command_buffer)
        {
            _M_current_command_buffer->setViewport(0, inverse_viewport());
        }

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

    VulkanAPI& VulkanAPI::wait_idle()
    {
        _M_device.waitIdle();
        return *this;
    }

    VulkanAPI& VulkanAPI::destroy_object(ObjID& ID)
    {
        if (ID)
        {
            delete OBJECT_OF(ID);
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::create_shader(ObjID& ID, const ShaderParams& params)
    {
        destroy_object(ID);

        VulkanShader* shader = new VulkanShader();
        try
        {
            shader->init(params);
            ID = shader->ID();
        }
        catch (const std::exception& ex)
        {
            delete shader;
            ID = 0;
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::use_shader(const ObjID& ID)
    {
        if (ID)
        {
            GET_TYPE(VulkanShader, ID)->use();
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::shader_value(const ObjID& ID, const String& name, void* data)
    {
        if (ID)
        {
            GET_TYPE(VulkanShader, ID)->set_value(name, data);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::generate_mesh(ObjID& ID)
    {
        destroy_object(ID);
        ID = (new VulkanMesh)->ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::mesh_data(const ObjID& ID, size_t size, DrawMode mode, void* data)
    {
        if (ID)
        {
            GET_TYPE(VulkanMesh, ID)->data(size, mode, data);
        }
        return *this;
    }


    VulkanAPI& VulkanAPI::draw_mesh(const ObjID& ID, Primitive primitive, size_t vertices, size_t offset)
    {
        if (ID)
        {
            GET_TYPE(VulkanMesh, ID)->draw(primitive, vertices, offset);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::update_mesh_data(const ObjID&, size_t, size_t, void*)
    {
        return *this;
    }

    VulkanAPI& VulkanAPI::mesh_indexes_array(const ObjID& ID, size_t size, const IndexBufferComponent& type, void* data)
    {
        if (ID)
        {
            GET_TYPE(VulkanMesh, ID)->index_buffer(size, type, data);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::create_texture(ObjID& ID, const TextureParams& params)
    {
        ID = (new VulkanTexture())->init(params).ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::gen_texture_2D(const ObjID& ID, const Size2D& size, int_t mipmap, void* data)
    {
        if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->gen_texture_2D(size, mipmap, data);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_texture(const ObjID& ID, TextureBindIndex binding)
    {
        if (ID && _M_current_shader)
        {
            _M_current_shader->bind_texture(GET_TYPE(VulkanTexture, ID), binding);
        }
        return *this;
    }
}// namespace Engine
