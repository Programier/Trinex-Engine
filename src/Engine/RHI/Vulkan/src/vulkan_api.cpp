#include <VkBootstrap.h>
#include <fstream>

#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <imgui_impl_vulkan.h>
#include <string>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_export.hpp>
#include <vulkan_object.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

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

    API_EXPORT RHI* load_api()
    {
        if (VulkanAPI::_M_vulkan == nullptr)
            VulkanAPI::_M_vulkan = new VulkanAPI();
        return VulkanAPI::_M_vulkan;
    }

    static vk::PresentModeKHR present_mode_of(bool vsync)
    {
        return vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
    }

    VulkanAPI::VulkanAPI()
    {

        _M_swap_chain_mode = present_mode_of(false);
        _M_state           = new VulkanState();
        _M_state->reset();
    }

    VulkanAPI::~VulkanAPI()
    {
        delete _M_state;
        _M_state = nullptr;
    }

    VulkanAPI& VulkanAPI::destroy_window()
    {
        _M_device.waitIdle();


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

    void* VulkanAPI::init_window(WindowInterface* window, const WindowConfig& config)
    {
        _M_window          = window;
        _M_swap_chain_mode = present_mode_of(config.vsync);

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
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* pUserData)
    {
        vulkan_error_log("Vulkan API", "%s", pCallbackData->pMessage);
        return VK_FALSE;
    }
#endif

    void VulkanAPI::init()
    {
        vkb::InstanceBuilder instance_builder;

        vulkan_info_log("VulkanAPI", "Enable extention %s", "Test");
        auto extentions = _M_window->required_extensions();
        for (auto& extension : extentions)
        {
            vulkan_info_log("VulkanAPI", "Enable extention %s", extension);
            instance_builder.enable_extension(extension);
        }

#if ENABLE_VALIDATION_LAYERS
        instance_builder.set_debug_callback(debug_callback).request_validation_layers();
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
        _M_renderer   = _M_properties.deviceName.data();


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
        void* _surface = _M_window->create_surface("", static_cast<VkInstance>(_M_instance));
        _M_surface     = vk::SurfaceKHR(*reinterpret_cast<VkSurfaceKHR*>(_surface));
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
        if (_M_need_recreate_swap_chain)
        {
            _M_need_recreate_swap_chain = false;
            wait_idle();
            destroy_framebuffers(false);
            create_swap_chain();
            create_framebuffers();
        }
    }


    void VulkanAPI::create_swap_chain()
    {
        auto size          = _M_window->size();
        window_data.width  = size.x;
        window_data.height = size.y;

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

        vk::ImageCreateInfo image_info(flags, vk::ImageType::e2D, texture->_M_vulkan_format,
                                       vk::Extent3D(texture->size.width, texture->size.height, 1),
                                       texture->_M_mipmap_count, layers, vk::SampleCountFlagBits::e1, tiling, usage,
                                       vk::SharingMode::eExclusive);

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
            _M_main_framebuffer = new VulkanMainFrameBuffer();

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

        auto size = _M_window->size();
        framebuffer(0)->size(static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));

        return *this;
    }

    std::size_t count_draw_calls = 0;
    VulkanAPI& VulkanAPI::swap_buffer()
    {
        _M_current_buffer = (_M_current_buffer + 1) % MAIN_FRAMEBUFFERS_COUNT;
        ++_M_current_frame;
        count_draw_calls = 0;
        return *this;
    }

    vk::ResultValue<uint32_t> VulkanAPI::swapchain_image_index()
    {
        static vk::ResultValue<uint32_t> result(vk::Result::eSuccess, 0);
        if (_M_need_update_image_index)
        {

            result = _M_device.acquireNextImageKHR(_M_swap_chain->_M_swap_chain, UINT64_MAX,
                                                   _M_command_buffer->sync_object()._M_available_semaphore, nullptr);

            _M_need_update_image_index = false;
        }
        return result;
    }


    VulkanAPI& VulkanAPI::begin_render()
    {
        _M_state->reset();
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

    VulkanAPI& VulkanAPI::vsync(bool flag)
    {
        _M_swap_chain_mode          = present_mode_of(flag);
        _M_need_recreate_swap_chain = true;
        return *this;
    }

    bool VulkanAPI::vsync()
    {
        return _M_swap_chain_mode == vk::PresentModeKHR::eFifo;
    }

    VulkanAPI& VulkanAPI::wait_idle()
    {
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

    VulkanAPI& VulkanAPI::draw_indexed(size_t indices, size_t offset)
    {
        _M_command_buffer->get().drawIndexed(indices, 1, offset, 0, 0);
        //++_M_state->_M_shader->_M_current_descriptor_index;
        //++count_draw_calls;

        return *this;
    }

    VulkanAPI& VulkanAPI::draw(size_t vertex_count)
    {
        _M_command_buffer->get().draw(vertex_count, 1, 0, 0);
        return *this;
    }

    String VulkanAPI::renderer()
    {
        return _M_renderer;
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

    bool VulkanAPI::check_format_support(ColorFormat format)
    {
        try
        {
            return static_cast<bool>(
                    _M_physical_device.getFormatProperties(parse_engine_format(format)).optimalTilingFeatures);
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
