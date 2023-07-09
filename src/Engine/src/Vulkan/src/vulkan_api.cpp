#include <Core/benchmark.hpp>
#include <VkBootstrap.h>
#include <fstream>
#include <iostream>

#include <imgui_impl_vulkan.h>
#include <string>
#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_export.hpp>
#include <vulkan_mesh.hpp>
#include <vulkan_object.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_ssbo.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>
#include <vulkan_uniform_buffer.hpp>


namespace Engine
{
    Vector<const char*> VulkanAPI::device_extensions = {
            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME,
    };

#if ENABLE_VALIDATION_LAYERS
    const Vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation",

    };
#endif

    VulkanAPI* VulkanAPI::_M_vulkan = nullptr;

    API_EXPORT GraphicApiInterface::ApiInterface* load_api()
    {
        if (VulkanAPI::_M_vulkan == nullptr)
            VulkanAPI::_M_vulkan = new VulkanAPI();
        return VulkanAPI::_M_vulkan;
    }

    VulkanObject::~VulkanObject()
    {}

    static VulkanUniformBufferBlock* uniform_buffer_allocator(std::size_t size)
    {
        VulkanUniformBufferBlock* ubo = new VulkanUniformBufferBlock();
        ubo->create(nullptr, size);
        return ubo;
    }


#define DEFAULT_PRESENT_MODE vk::PresentModeKHR::eImmediate
    VulkanAPI::VulkanAPI()
    {
        _M_uniform_allocator.allocator(uniform_buffer_allocator);
        _M_swap_chain_mode = DEFAULT_PRESENT_MODE;
#if USE_THREADED_END_COMMAND
        _M_enabled_threaded_end_command.store(true);
#endif
    }

    VulkanAPI::~VulkanAPI()
    {}

    VulkanAPI& VulkanAPI::destroy_window()
    {
        _M_device.waitIdle();

        _M_uniform_allocator.destroy();

        delete _M_command_buffer;
        _M_command_buffer = nullptr;
        destroy_framebuffers();
        delete _M_swap_chain;

        _M_device.destroyCommandPool(_M_command_pool);

        _M_device.destroy();


        vk::Instance(_M_instance.instance).destroySurfaceKHR(_M_surface);
        vkb::destroy_instance(_M_instance);
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_init()
    {
        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance       = _M_instance;
        init_info.PhysicalDevice = _M_physical_device;
        init_info.Device         = _M_device;
        init_info.QueueFamily    = _M_graphics_and_present_index.graphics_family.value();
        init_info.Queue          = _M_graphics_queue;

        VkDescriptorPoolSize pool_sizes[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000},
        };

        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
        descriptor_pool_create_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets                    = 1000;
        descriptor_pool_create_info.poolSizeCount              = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
        descriptor_pool_create_info.pPoolSizes                 = pool_sizes;

        _M_imgui_descriptor_pool = _M_device.createDescriptorPool(descriptor_pool_create_info);
        init_info.DescriptorPool = _M_imgui_descriptor_pool;

        init_info.MinImageCount = _M_swap_chain->_M_bootstrap_swapchain.requested_min_image_count;
        init_info.ImageCount    = _M_swap_chain->_M_bootstrap_swapchain.image_count;

        ImGui_ImplVulkan_Init(&init_info, _M_main_framebuffer->_M_render_pass);

        auto command_buffer = begin_single_time_command_buffer();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        end_single_time_command_buffer(command_buffer);

        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_terminate()
    {
        wait_idle();
        DESTROY_CALL(destroyDescriptorPool, _M_imgui_descriptor_pool);
        ImGui_ImplVulkan_Shutdown();
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_new_frame()
    {
        ImGui_ImplVulkan_NewFrame();
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_render()
    {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _M_command_buffer->get());
        return *this;
    }

    vk::Device* vulkan_device()
    {
        return &API->_M_device;
    }

    vk::CommandPool* vulkan_command_pool()
    {
        return &API->_M_command_pool;
    }

    void* VulkanAPI::init_window(SDL_Window* window)
    {
        _M_window  = window;
        auto funcs = {
                &VulkanAPI::init,
                &VulkanAPI::create_command_buffer,
                &VulkanAPI::create_swap_chain,
                &VulkanAPI::create_framebuffers,
        };

        try
        {
            for (auto func : funcs) (this->*func)();
        }
        catch (const std::exception& e)
        {
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
        vulkan_debug_log("Vulkan API", "%s", pCallbackData->pMessage);
        return VK_FALSE;
    }
#endif

    static Vector<const char*> get_required_extensions(SDL_Window* window)
    {
        uint32_t count = 0;

        SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
        Vector<const char*> extensions(count);

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
#else
        instance_builder.add_validation_disable(VK_VALIDATION_CHECK_ALL_EXT);
        instance_builder.add_validation_feature_disable(VK_VALIDATION_FEATURE_DISABLE_ALL_EXT);
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

        _M_properties = _M_physical_device.getProperties();
        _M_renderer   = _M_properties.deviceName._M_elems;


        vkb::DeviceBuilder device_builder(phys_device_ret.value());
        vk::PhysicalDeviceIndexTypeUint8FeaturesEXT idx_byte_feature(VK_TRUE);
        device_builder.add_pNext(&idx_byte_feature);


        auto device_ret = device_builder.build();
        if (!device_ret)
        {
            throw std::runtime_error(device_ret.error().message());
        }

        _M_bootstrap_device = device_ret.value();
        _M_device           = vk::Device(device_ret.value().device);

        auto index_1        = _M_bootstrap_device.get_queue_index(vkb::QueueType::graphics);
        auto index_2        = _M_bootstrap_device.get_queue_index(vkb::QueueType::present);
        auto graphics_queue = _M_bootstrap_device.get_queue(vkb::QueueType::graphics);
        auto present_queue  = _M_bootstrap_device.get_queue(vkb::QueueType::present);

        if (!index_1 || !index_2 || !graphics_queue || !present_queue)
        {
            throw std::runtime_error("Failed to init queues");
        }

        _M_graphics_and_present_index.graphics_family = index_1.value();
        _M_graphics_and_present_index.present_family  = index_2.value();

        _M_graphics_queue = vk::Queue(graphics_queue.value());
        _M_present_queue  = vk::Queue(present_queue.value());
    }


    void VulkanAPI::create_surface()
    {
        VkSurfaceKHR _surface;
        SDL_Vulkan_CreateSurface(_M_window, static_cast<VkInstance>(_M_instance), &_surface);

        _M_surface = vk::SurfaceKHR(_surface);
    }

    void VulkanAPI::destroy_framebuffers(bool full_destroy)
    {
        if (full_destroy)
        {
            delete _M_main_framebuffer;
            return;
        }

        _M_main_framebuffer->destroy();
    }

    void VulkanAPI::recreate_swap_chain()
    {
        if (_M_need_recreate_swap_chain && API->_M_active_threads.load() == 0)
        {
            _M_need_recreate_swap_chain = false;
            wait_idle();
            destroy_framebuffers(false);
            create_swap_chain();
            create_framebuffers();
#if USE_THREADED_END_COMMAND
            API->_M_enabled_threaded_end_command.store(true);
#endif
        }
    }


    void VulkanAPI::create_swap_chain()
    {
        SDL_Vulkan_GetDrawableSize(_M_window, &window_data.width, &window_data.height);
        if (_M_swap_chain)
            delete _M_swap_chain;

        _M_swap_chain = new SwapChain();
    }

    VulkanFramebuffer* VulkanAPI::init_base_framebuffer_renderpass(VulkanFramebuffer* framebuffer)
    {
        framebuffer->_M_attachment_descriptions.resize(1);
        vk::Format color_format = _M_swap_chain->_M_format;

        framebuffer->_M_attachment_descriptions[0] = vk::AttachmentDescription(
                vk::AttachmentDescriptionFlags(), color_format, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);


        framebuffer->_M_color_attachment_references = {
                vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
        };

        return framebuffer;
    }

    vk::Format VulkanAPI::find_supported_format(const Vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                                vk::FormatFeatureFlags features)
    {
        for (const vk::Format& format : candidates)
        {
            vk::FormatProperties properties = _M_physical_device.getFormatProperties(format);

            if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        throw std::runtime_error("VulkanAPI: Failed to find supported format!");
    }


    bool VulkanAPI::has_stencil_component(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    vk::Format VulkanAPI::find_depth_format()
    {
        static const Vector<vk::Format> candidates = {
                vk::Format::eD32Sfloat,
                vk::Format::eD32SfloatS8Uint,
                vk::Format::eD24UnormS8Uint,
        };

        return find_supported_format(candidates, vk::ImageTiling::eOptimal,
                                     vk::FormatFeatureFlagBits::eDepthStencilAttachment);
    }

    VulkanAPI& VulkanAPI::create_resolve_image(uint32_t width, uint32_t height, vk::Format format,
                                               vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                               vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                                               vk::Image& image, vk::DeviceMemory& image_memory, uint32_t mip_count,
                                               uint32_t layers)
    {
        vk::ImageCreateInfo image_info(flags, vk::ImageType::e2D, format, vk::Extent3D(width, height, 1), mip_count,
                                       layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive);

        image                                      = API->_M_device.createImage(image_info);
        vk::MemoryRequirements memory_requirements = API->_M_device.getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo alloc_info(
                memory_requirements.size,
                API->find_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent));
        image_memory = API->_M_device.allocateMemory(alloc_info);
        API->_M_device.bindImageMemory(image, image_memory, 0);
        return *this;
    }

    VulkanAPI& VulkanAPI::create_image(VulkanTexture* texture, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                       vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                                       vk::DeviceMemory& image_memory, uint32_t layers)
    {
        VulkanTextureState* state = &texture->state;

        vk::ImageCreateInfo image_info(flags, vk::ImageType::e2D, state->format,
                                       vk::Extent3D(state->size.width, state->size.height, 1), state->mipmap_count,
                                       layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive);

        image                                      = API->_M_device.createImage(image_info);
        vk::MemoryRequirements memory_requirements = API->_M_device.getImageMemoryRequirements(image);
        vk::MemoryAllocateInfo alloc_info(
                memory_requirements.size,
                API->find_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent));
        image_memory = API->_M_device.allocateMemory(alloc_info);
        API->_M_device.bindImageMemory(image, image_memory, 0);
        return *this;
    }


    void VulkanAPI::create_framebuffers()
    {
        if (!_M_main_framebuffer)
            _M_main_framebuffer = new VulkanFramebuffer();

        _M_main_framebuffer->_M_buffers.clear();
        _M_main_framebuffer->_M_buffers.resize(_M_swap_chain->_M_images.size());


        init_base_framebuffer_renderpass(_M_main_framebuffer);
        _M_main_framebuffer->_M_is_custom   = false;
        _M_main_framebuffer->_M_size.height = _M_swap_chain->_M_extent.height;
        _M_main_framebuffer->_M_size.width  = _M_swap_chain->_M_extent.width;

        size_t index = 0;
        for (auto const& image_view : _M_swap_chain->_M_image_views)
        {
            _M_main_framebuffer->_M_buffers[index]._M_attachments.resize(1);
            _M_main_framebuffer->_M_buffers[index]._M_attachments[0] = image_view;
            index++;
        }

        _M_main_framebuffer->create_framebuffer();
    }

    void VulkanAPI::create_command_buffer()
    {
        _M_command_pool = _M_device.createCommandPool(
                vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                          _M_graphics_and_present_index.graphics_family.value()));

        _M_command_buffer = new VulkanCommandBuffer();
    }


    VulkanAPI& VulkanAPI::on_window_size_changed()
    {
        _M_need_recreate_swap_chain = true;

        int w, h;
        SDL_Vulkan_GetDrawableSize(_M_window, &w, &h);
        framebuffer(0)->size(static_cast<uint32_t>(w), static_cast<uint32_t>(h));

        return *this;
    }

    std::size_t count_draw_calls = 0;
    VulkanAPI& VulkanAPI::swap_buffer(SDL_Window*)
    {
        _M_current_frame = (_M_current_frame + 1) % MAIN_FRAMEBUFFERS_COUNT;
        _M_next_frame    = (_M_next_frame + 1) % MAIN_FRAMEBUFFERS_COUNT;
        count_draw_calls = 0;
        return *this;
    }

    vk::ResultValue<uint32_t> VulkanAPI::swapchain_image_index()
    {
        static vk::ResultValue<uint32_t> result(vk::Result::eSuccess, 0);
        if (_M_need_update_image_index)
        {

            result = _M_device.acquireNextImageKHR(_M_swap_chain->_M_swap_chain, UINT32_MAX,
                                                   _M_command_buffer->sync_object()._M_available_semaphore, nullptr);

            _M_need_update_image_index = false;
        }
        return result;
    }


    VulkanAPI& VulkanAPI::begin_render()
    {
        _M_command_buffer->begin();
        return *this;
    }

    VulkanAPI& VulkanAPI::end_render()
    {
        _M_command_buffer->end();
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

    VulkanAPI& VulkanAPI::framebuffer_viewport(const Identifier& ID, const ViewPort& viewport)
    {
        framebuffer(ID)->update_viewport(viewport);
        return *this;
    }

    static Logger** get_default_logger()
    {
        static Logger default_vulkan_logger;
        static Logger* default_vulkan_logger_ptr = &default_vulkan_logger;
        return &default_vulkan_logger_ptr;
    }


    VulkanAPI& VulkanAPI::logger(Logger*& logger)
    {
        if (logger)
        {
            _M_engine_logger = &logger;
        }
        else
        {
            _M_engine_logger = get_default_logger();
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_framebuffer(const Identifier& ID, size_t buffer_index)
    {
        VulkanFramebuffer* buffer = framebuffer(ID);
        if (buffer == _M_main_framebuffer)
        {
            auto index = swapchain_image_index().value;
            buffer->bind(index);
        }
        else
        {
            buffer->bind(buffer_index);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::clear_color(const Identifier& ID, const ColorClearValue& color, byte layout)
    {
        framebuffer(ID)->clear_color(color, layout);
        return *this;
    }

    VulkanAPI& VulkanAPI::swap_interval(int_t interval)
    {
        static const Map<int_t, vk::PresentModeKHR> modes = {{-1, vk::PresentModeKHR::eFifoRelaxed},
                                                             {0, DEFAULT_PRESENT_MODE},
                                                             {1, vk::PresentModeKHR::eFifo}};
        int_t index                                       = interval != 0 ? interval / glm::abs(interval) : interval;
        interval                                          = glm::abs(interval);
        _M_swap_chain_mode                                = modes.at(index);
        _M_need_recreate_swap_chain                       = true;
        return *this;
    }

    VulkanAPI& VulkanAPI::wait_idle()
    {
        if (_M_command_buffer)
        {
            for (auto* command : _M_command_buffer->_M_end_commands)
            {
                command->wait();
            }
        }
        _M_device.waitIdle();
        return *this;
    }

    VulkanAPI& VulkanAPI::destroy_object(Identifier& ID)
    {
        // if (ID)
        {
            delete OBJECT_OF(ID);
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::create_shader(Identifier& ID, const PipelineCreateInfo& params)
    {
        destroy_object(ID);

        VulkanShader* shader = new VulkanShader();

        if (shader->init(params))
        {
            ID = shader->ID();
        }
        else
        {
            delete shader;
            ID = 0;
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::use_shader(const Identifier& ID)
    {
        GET_TYPE(VulkanShader, ID)->use();
        return *this;
    }


    VulkanAPI& VulkanAPI::create_vertex_buffer(Identifier& ID, const byte* data, size_t size)
    {
        ID = (new VulkanVertexBuffer())->create(data, size).ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::update_vertex_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        // if (ID)
        {
            GET_TYPE(VulkanVertexBuffer, ID)->update(offset, data, size);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_vertex_buffer(const Identifier& ID, size_t offset)
    {
        // if (ID)
        {
            GET_TYPE(VulkanVertexBuffer, ID)->bind(offset);
        }
        return *this;
    }

    MappedMemory VulkanAPI::map_vertex_buffer(const Identifier& ID)
    {
        return GET_TYPE(VulkanVertexBuffer, ID)->map_memory();
    }

    VulkanAPI& VulkanAPI::unmap_vertex_buffer(const Identifier& ID)
    {
        GET_TYPE(VulkanVertexBuffer, ID)->unmap_memory();
        return *this;
    }

    VulkanAPI& VulkanAPI::create_index_buffer(Identifier& ID, const byte* data, size_t size,
                                              IndexBufferComponent component)
    {
        ID = (new VulkanIndexBuffer())->create(data, size, component).ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::update_index_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        // if (ID)
        {
            GET_TYPE(VulkanIndexBuffer, ID)->update(offset, data, size);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_index_buffer(const Identifier& ID, size_t offset)
    {
        // if (ID)
        {
            GET_TYPE(VulkanIndexBuffer, ID)->bind(offset);
        }
        return *this;
    }

    MappedMemory VulkanAPI::map_index_buffer(const Identifier& ID)
    {
        return GET_TYPE(VulkanIndexBuffer, ID)->map_memory();
    }

    VulkanAPI& VulkanAPI::unmap_index_buffer(const Identifier& ID)
    {
        GET_TYPE(VulkanIndexBuffer, ID)->unmap_memory();
        return *this;
    }


    VulkanAPI& VulkanAPI::draw_indexed(size_t indices, size_t offset)
    {
        _M_command_buffer->get().drawIndexed(indices, 1, offset, 0, 0);
        _M_command_buffer->get().drawIndexed(indices, 1, offset, 0, 0);
        ++current_shader()->_M_current_descriptor_index;
        ++count_draw_calls;

        return *this;
    }

    VulkanAPI& VulkanAPI::create_texture(Identifier& ID, const TextureCreateInfo& params, TextureType type)
    {
        ID = (new VulkanTexture())->init(params, type).ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_texture(const Identifier& ID, TextureBindIndex binding)
    {
        current_shader()->bind_texture(GET_TYPE(VulkanTexture, ID), binding);
        return *this;
    }

    VulkanAPI& VulkanAPI::swizzle_texture(const Identifier& ID, const SwizzleRGBA& swizzle)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->swizzle(swizzle);
        }
        return *this;
    }

    SwizzleRGBA VulkanAPI::swizzle_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->swizzle();
        }
        return SwizzleRGBA();
    }

    VulkanAPI& VulkanAPI::wrap_s_texture(const Identifier& ID, const WrapValue& value)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_s(value);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::wrap_t_texture(const Identifier& ID, const WrapValue& value)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_t(value);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::wrap_r_texture(const Identifier& ID, const WrapValue& value)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_r(value);
        }
        return *this;
    }

    WrapValue VulkanAPI::wrap_s_texture(const Identifier& ID)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_s();
        }

        return WrapValue();
    }

    WrapValue VulkanAPI::wrap_t_texture(const Identifier& ID)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_t();
        }

        return WrapValue();
    }

    WrapValue VulkanAPI::wrap_r_texture(const Identifier& ID)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->wrap_r();
        }

        return WrapValue();
    }

    VulkanAPI& VulkanAPI::min_filter_texture(const Identifier& ID, TextureFilter filter)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->min_filter(filter);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::mag_filter_texture(const Identifier& ID, TextureFilter filter)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->mag_filter(filter);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::compare_func_texture(const Identifier& ID, CompareFunc func)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->compare_func(func);
        }
        return *this;
    }

    CompareFunc VulkanAPI::compare_func_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->compare_func();
        }

        return CompareFunc();
    }

    VulkanAPI& VulkanAPI::compare_mode_texture(const Identifier& ID, CompareMode mode)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->compare_mode(mode);
        }
        return *this;
    }

    TextureFilter VulkanAPI::min_filter_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->mag_filter();
        }

        return TextureFilter();
    }

    TextureFilter VulkanAPI::mag_filter_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->mag_filter();
        }
        return TextureFilter();
    }

    CompareMode VulkanAPI::compare_mode_texture(const Identifier& ID)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->compare_mode();
        }

        return CompareMode::None;
    }

    MipMapLevel VulkanAPI::base_level_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.base_mip_level;
        }
        return 0;
    }

    VulkanAPI& VulkanAPI::base_level_texture(const Identifier& ID, MipMapLevel level)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->base_level(level);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::min_lod_level_texture(const Identifier& ID, LodLevel level)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->min_lod_level(level);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::max_lod_level_texture(const Identifier& ID, LodLevel level)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->max_lod_level(level);
        }
        return *this;
    }

    LodLevel VulkanAPI::min_lod_level_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.min_lod;
        }
        return LodLevel();
    }

    LodLevel VulkanAPI::max_lod_level_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.max_lod;
        }
        return LodLevel();
    }

    MipMapLevel VulkanAPI::max_mipmap_level_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.mipmap_count;
        }
        return LodLevel();
    }


    VulkanAPI& VulkanAPI::texture_size(const Identifier& ID, Size2D& size, MipMapLevel level)
    {
        // if (ID)
        {
            size = GET_TYPE(VulkanTexture, ID)->size(level);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::generate_texture_mipmap(const Identifier& ID)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->generate_mipmap();
        }

        return *this;
    }

    String VulkanAPI::renderer()
    {
        return _M_renderer;
    }

    VulkanAPI& VulkanAPI::read_texture_2D_data(const Identifier& ID, Vector<byte>& data, MipMapLevel level)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->read_texture_2D_data(data, level);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::update_texture_2D(const Identifier& ID, const Size2D& size, const Offset2D& offset,
                                            MipMapLevel level, const void* data)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->update_texture_2D(size, offset, level, 0, data);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::anisotropic_filtering_texture(const Identifier& ID, float value)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->anisotropic_value(value);
        }
        return *this;
    }

    float VulkanAPI::anisotropic_filtering_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.anisotropy;
        }

        return 1.0;
    }

    float VulkanAPI::max_anisotropic_filtering()
    {
        static float value = 0.0f;
        if (value < 1.f)
        {
            value = _M_properties.limits.maxSamplerAnisotropy;
            if (value < 1.0f)
                value = 1.0f;
        }
        return value;
    }

    VulkanAPI& VulkanAPI::cubemap_texture_update_data(const Identifier& ID, TextureCubeMapFace face, const Size2D& size,
                                                      const Offset2D& offset, MipMapLevel mipmap, void* data)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)
                    ->update_texture_2D(size, offset, mipmap, static_cast<EnumerateType>(face), data);
        }
        return *this;
    }

    SamplerMipmapMode VulkanAPI::sample_mipmap_mode_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->sample_mipmap_mode_texture();
        }
        return SamplerMipmapMode();
    }

    VulkanAPI& VulkanAPI::sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->sample_mipmap_mode_texture(mode);
        }
        return *this;
    }

    LodBias VulkanAPI::lod_bias_texture(const Identifier& ID)
    {
        // if (ID)
        {
            return GET_TYPE(VulkanTexture, ID)->state.mip_lod_bias;
        }
        return 0.0;
    }

    VulkanAPI& VulkanAPI::lod_bias_texture(const Identifier& ID, LodBias bias)
    {
        // if (ID)
        {
            GET_TYPE(VulkanTexture, ID)->lod_bias_texture(glm::min(bias, max_lod_bias_texture()));
        }
        return *this;
    }

    LodBias VulkanAPI::max_lod_bias_texture()
    {
        return _M_properties.limits.maxSamplerLodBias;
    }

    VulkanAPI& VulkanAPI::gen_framebuffer(Identifier& ID, const FrameBufferCreateInfo& info)
    {
        ID = (new VulkanFramebuffer())->init(info).ID();
        return *this;
    }

    Identifier VulkanAPI::imgui_texture_id(const Identifier&)
    {
        throw std::runtime_error(not_implemented);
    }

    VulkanFramebuffer* VulkanAPI::framebuffer(Identifier ID)
    {
        if (ID)
            return GET_TYPE(VulkanFramebuffer, ID);

        return _M_main_framebuffer;
    }

    VulkanAPI& VulkanAPI::create_ssbo(Identifier& ID, const byte* data, size_t size)
    {
        ID = (new VulkanSSBO())->create(data, size).ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::update_ssbo(const Identifier& ID, const byte* data, size_t offset, size_t size)
    {
        GET_TYPE(VulkanSSBO, ID)->update(offset, data, size);
        return *this;
    }

    VulkanShader* VulkanAPI::current_shader()
    {
        return _M_command_buffer->state()._M_current_shader;
    }

    VulkanAPI& VulkanAPI::bind_ssbo(const Identifier& ID, BindingIndex index, size_t offset, size_t size)
    {
        current_shader()->bind_shared_buffer(GET_TYPE(VulkanSSBO, ID), offset, size, index);
        return *this;
    }

    VulkanAPI& VulkanAPI::create_uniform_buffer(Identifier& ID, const byte* data, size_t size)
    {
        ID = (new VulkanUniformBufferMap(data, size))->ID();
        return *this;
    }

    VulkanAPI& VulkanAPI::update_uniform_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        GET_TYPE(VulkanUniformBufferMap, ID)->next_buffer()->update(offset, data, size);
        return *this;
    }

    VulkanAPI& VulkanAPI::bind_uniform_buffer(const Identifier& ID, BindingIndex index)
    {
        current_shader()->bind_ubo(GET_TYPE(VulkanUniformBufferMap, ID)->current_buffer(), index);
        return *this;
    }

    MappedMemory VulkanAPI::map_uniform_buffer(const Identifier& ID)
    {
        return GET_TYPE(VulkanUniformBufferMap, ID)->next_buffer()->map_memory();
    }

    VulkanAPI& VulkanAPI::unmap_uniform_buffer(const Identifier& ID)
    {
        GET_TYPE(VulkanUniformBufferMap, ID)->next_buffer()->unmap_memory();
        return *this;
    }


    VulkanAPI& VulkanAPI::framebuffer_scissor(const Identifier& ID, const Scissor& scissor)
    {
        framebuffer(ID)->update_scissor(scissor);
        return *this;
    }

    VulkanAPI& VulkanAPI::clear_depth_stencil(const Identifier& ID, const DepthStencilClearValue& value)
    {
        framebuffer(ID)->clear_depth_stencil(value);
        return *this;
    }

    bool VulkanAPI::check_format_support(PixelType type, PixelComponentType component)
    {
        try
        {
            return static_cast<bool>(
                    _M_physical_device.getFormatProperties(VulkanTexture::parse_format(type, component))
                            .optimalTilingFeatures);
        }
        catch (...)
        {
            return false;
        }
    }

    VulkanAPI& VulkanAPI::async_render(bool flag)
    {
        return *this;
    }

    bool VulkanAPI::async_render()
    {
        return false;
    }

    VulkanAPI& VulkanAPI::next_render_thread()
    {
        return *this;
    }
}// namespace Engine
