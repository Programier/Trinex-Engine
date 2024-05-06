#include <VkBootstrap.h>
#include <fstream>

#include <Graphics/texture.hpp>
#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <imgui_impl_vulkan.h>
#include <string>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_export.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>
#include <vulkan_uniform_buffer.hpp>
#include <vulkan_viewport.hpp>

#define VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT 2
#define VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT 3

namespace Engine
{
#if ENABLE_VALIDATION_LAYERS
    const Vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation",
    };
#endif

    VulkanAPI* VulkanAPI::m_vulkan = nullptr;

    TRINEX_EXTERNAL_LIB_INIT_FUNC(RHI*)
    {
        if (VulkanAPI::m_vulkan == nullptr)
            VulkanAPI::m_vulkan = new VulkanAPI();
        return VulkanAPI::m_vulkan;
    }


    static constexpr inline size_t ext_maintenance1_index     = 0;
    static constexpr inline size_t ext_swapchain_index        = 1;
    static constexpr inline size_t ext_index_type_uint8_index = 2;


    static constexpr inline size_t ext_count = 3;

    VulkanAPI::VulkanAPI()
    {
        m_device_extensions.resize(ext_count);

        m_device_extensions[ext_maintenance1_index]     = {VK_KHR_MAINTENANCE1_EXTENSION_NAME, true, false};
        m_device_extensions[ext_swapchain_index]        = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, false};
        m_device_extensions[ext_index_type_uint8_index] = {VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, true, false};

        m_state = new VulkanState();
    }

    VulkanAPI::~VulkanAPI()
    {
        wait_idle();
        for (VulkanUniformBuffer* buffer : m_uniform_buffer)
        {
            delete buffer;
        }

        m_uniform_buffer.clear();

        delete_garbage(true);
        delete m_main_render_pass;

        DESTROY_CALL(destroyDescriptorPool, m_imgui_descriptor_pool);
        m_device.destroyCommandPool(m_command_pool);
        m_device.destroy();
        vkb::destroy_instance(m_instance);
        delete m_state;
    }

    vk::PresentModeKHR VulkanAPI::present_mode_of(bool vsync)
    {
        return vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
    }

    bool VulkanAPI::vsync_from_present_mode(vk::PresentModeKHR present_mode)
    {
        return present_mode == vk::PresentModeKHR::eFifo ? true : false;
    }

    VulkanAPI& VulkanAPI::delete_garbage(bool force)
    {
        if (force)
        {
            for (auto& ell : m_garbage)
            {
                delete ell.object;
            }

            m_garbage.clear();
        }
        else
        {
            while (!m_garbage.empty())
            {
                Garbage* garbage = &m_garbage.front();
                if (garbage->frame != m_current_frame)
                    break;

                delete garbage->object;
                m_garbage.pop_front();
            }
        }

        return *this;
    }

    VulkanAPI& VulkanAPI::destroy_object(RHI_Object* object)
    {
        if (object->internal_type() > VULKAN_FORCED_DESTROY_TYPES)
        {
            delete object;
        }
        else
        {
            m_garbage.emplace_back(object, m_current_frame + m_framebuffers_count + 1);
        }
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_init(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.Instance       = m_instance;
        init_info.PhysicalDevice = m_physical_device;
        init_info.Device         = m_device;
        init_info.QueueFamily    = m_graphics_and_present_index.graphics_family.value();
        init_info.Queue          = m_graphics_queue;

        if (!m_imgui_descriptor_pool)
        {
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
            descriptor_pool_create_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            m_imgui_descriptor_pool = m_device.createDescriptorPool(descriptor_pool_create_info);
        }

        init_info.DescriptorPool = m_imgui_descriptor_pool;

        init_info.MinImageCount = m_framebuffers_count;
        init_info.ImageCount    = m_framebuffers_count;

        ImGui_ImplVulkan_Init(&init_info, m_main_render_pass->m_render_pass);


        ImGui_ImplVulkan_NewFrame();

        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_terminate(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplVulkan_Shutdown();
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_new_frame(ImGuiContext* ctx)
    {
        return *this;
    }

    VulkanAPI& VulkanAPI::imgui_render(ImGuiContext* ctx, ImDrawData* draw_data)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplVulkan_RenderDrawData(draw_data, current_command_buffer());
        return *this;
    }

    vk::Device* vulkan_device()
    {
        return &API->m_device;
    }

    vk::CommandPool* vulkan_command_pool()
    {
        return &API->m_command_pool;
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

    static bool is_available_swapchain_images_count(uint32_t count)
    {
        auto& m_surface_capabilities = API->m_surface_capabilities;
        return (m_surface_capabilities.maxImageCount == 0 || m_surface_capabilities.maxImageCount >= count) &&
               m_surface_capabilities.minImageCount <= count;
    }

    void VulkanAPI::initialize(WindowInterface* window)
    {
        m_window = window;

        vkb::InstanceBuilder instance_builder;
        instance_builder.require_api_version(VK_API_VERSION_1_3);

        auto extentions = m_window->required_extensions();
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

        m_instance = instance_ret.value();

        m_surface = create_surface(window);

        vkb::PhysicalDeviceSelector phys_device_selector(instance_ret.value());
        vk::PhysicalDeviceFeatures features;
        features.samplerAnisotropy  = true;
        features.fillModeNonSolid   = true;
        features.wideLines          = true;
        features.tessellationShader = true;
        features.geometryShader     = true;

        phys_device_selector.set_required_features(static_cast<VkPhysicalDeviceFeatures>(features));

        for (VulkanExtention& extension : m_device_extensions)
        {
            if (extension.required)
                phys_device_selector.add_required_extension(extension.name);
            else
                phys_device_selector.add_desired_extension(extension.name);
        }


        phys_device_selector.allow_any_gpu_device_type(false);
#if USE_INTEGRATED_GPU
        phys_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::integrated);
#else
        phys_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);
#endif
        phys_device_selector.set_surface(static_cast<VkSurfaceKHR>(m_surface));

        auto selected_device = phys_device_selector.select();
        if (!selected_device.has_value())
        {
            throw std::runtime_error(selected_device.error().message());
        }

        m_physical_device = vk::PhysicalDevice(selected_device.value().physical_device);
        check_extentions();

        m_properties           = m_physical_device.getProperties();
        m_features             = m_physical_device.getFeatures();
        m_surface_capabilities = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);
        m_renderer             = m_properties.deviceName.data();

        vkb::DeviceBuilder device_builder(selected_device.value());

        /// FEATURES
        vk::PhysicalDeviceIndexTypeUint8FeaturesEXT idx_byte_feature(VK_TRUE);
        device_builder.add_pNext(&idx_byte_feature);
        vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures separate_depth_stencil(VK_TRUE);
        device_builder.add_pNext(&separate_depth_stencil);


        auto device_ret = device_builder.build();
        if (!device_ret)
        {
            throw std::runtime_error(device_ret.error().message());
        }

        m_bootstrap_device = device_ret.value();
        m_device           = vk::Device(device_ret.value().device);

        auto index_1        = m_bootstrap_device.get_queue_index(vkb::QueueType::graphics);
        auto index_2        = m_bootstrap_device.get_queue_index(vkb::QueueType::present);
        auto graphics_queue = m_bootstrap_device.get_queue(vkb::QueueType::graphics);
        auto present_queue  = m_bootstrap_device.get_queue(vkb::QueueType::present);

        if (!index_1 || !index_2 || !graphics_queue || !present_queue)
        {
            throw std::runtime_error("Failed to init queues");
        }

        m_graphics_and_present_index.graphics_family = index_1.value();
        m_graphics_and_present_index.present_family  = index_2.value();

        m_graphics_queue = vk::Queue(graphics_queue.value());
        m_present_queue  = vk::Queue(present_queue.value());


        initialize_pfn();
        enable_dynamic_states();
        create_command_pool();

        if (is_available_swapchain_images_count(VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT))
        {
            m_framebuffers_count = VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT;
        }
        else if (is_available_swapchain_images_count(VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT))
        {
            m_framebuffers_count = VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT;
        }
        else if (m_surface_capabilities.minImageCount >= VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT)
        {
            m_framebuffers_count = m_surface_capabilities.minImageCount;
        }
        else
        {
            throw EngineException("Vulkan requires a minimum of 2 images for Swapchain");
        }

        m_uniform_buffer.resize(m_framebuffers_count);

        for (VulkanUniformBuffer*& buffer : m_uniform_buffer)
        {
            buffer = new VulkanUniformBuffer();
        }
    }

    void VulkanAPI::check_extentions()
    {
        auto properties = m_physical_device.enumerateDeviceExtensionProperties();
        for (VulkanExtention& extension : m_device_extensions)
        {
            for (auto& prop : properties)
            {
                if (std::strcmp(extension.name, prop.extensionName) == 0)
                {
                    extension.enabled = true;
                    break;
                }
            }
        }
    }

    void VulkanAPI::enable_dynamic_states()
    {
        m_dynamic_states = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eScissor,
        };
    }

    void VulkanAPI::initialize_pfn()
    {
        pfn.vkCmdBeginDebugUtilsLabelEXT =
                (PFN_vkCmdBeginDebugUtilsLabelEXT) vkGetDeviceProcAddr(m_device, "vkCmdBeginDebugUtilsLabelEXT");
        pfn.vkCmdEndDebugUtilsLabelEXT =
                (PFN_vkCmdEndDebugUtilsLabelEXT) vkGetDeviceProcAddr(m_device, "vkCmdEndDebugUtilsLabelEXT");
    }


    vk::SurfaceKHR VulkanAPI::create_surface(WindowInterface* interface)
    {
        void* _surface = interface->create_api_context("", static_cast<VkInstance>(m_instance));
        interface->bind_api_context(_surface);
        return vk::SurfaceKHR(*reinterpret_cast<VkSurfaceKHR*>(_surface));
    }

    vk::Extent2D VulkanAPI::surface_size() const
    {
        return surface_size(m_surface);
    }

    vk::Extent2D VulkanAPI::surface_size(const vk::SurfaceKHR& surface) const
    {
        return m_physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
    }

    bool VulkanAPI::has_stencil_component(vk::Format format)
    {
        return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
    }

    VulkanAPI& VulkanAPI::create_image(VulkanTexture* texture, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                       vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                                       vk::DeviceMemory& image_memory, uint32_t layers)
    {

        vk::ImageCreateInfo image_info(flags, vk::ImageType::e2D, texture->format(),
                                       vk::Extent3D(texture->size().x, texture->size().y, 1), texture->mipmap_count(), layers,
                                       vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive);

        image                                      = API->m_device.createImage(image_info);
        vk::MemoryRequirements memory_requirements = API->m_device.getImageMemoryRequirements(image);
        auto memory_type = API->find_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::MemoryAllocateInfo alloc_info(memory_requirements.size, memory_type);
        image_memory = API->m_device.allocateMemory(alloc_info);
        API->m_device.bindImageMemory(image, image_memory, 0);
        return *this;
    }

    void VulkanAPI::create_command_pool()
    {
        m_command_pool = m_device.createCommandPool(vk::CommandPoolCreateInfo(
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_graphics_and_present_index.graphics_family.value()));
    }


    VulkanAPI& VulkanAPI::begin_render()
    {
        delete_garbage(false);
        uniform_buffer()->reset();
        return *this;
    }

    VulkanAPI& VulkanAPI::end_render()
    {
        if (m_state->m_current_viewport)
        {
            m_state->m_current_viewport->end_render();
        }
        ++m_current_frame;
        m_current_buffer = m_current_frame % m_framebuffers_count;
        return *this;
    }

    uint32_t VulkanAPI::find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties)
    {
        vk::PhysicalDeviceMemoryProperties mem_properties = m_physical_device.getMemoryProperties();

        for (uint32_t i = 0; (1u << i) <= type_filter && i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1u << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties))
            {
                return i;
            }
        }

        throw std::runtime_error("VulkanAPI: Failed to find suitable memory type!");
    }

    VulkanAPI& VulkanAPI::create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                                        vk::Buffer& buffer, vk::DeviceMemory& buffer_memory)
    {
        vk::BufferCreateInfo buffer_info({}, size, usage, vk::SharingMode::eExclusive);
        buffer = m_device.createBuffer(buffer_info);

        vk::MemoryRequirements mem_requirements = m_device.getBufferMemoryRequirements(buffer);
        vk::MemoryAllocateInfo alloc_info(mem_requirements.size, find_memory_type(mem_requirements.memoryTypeBits, properties));


        buffer_memory = m_device.allocateMemory(alloc_info);
        m_device.bindBufferMemory(buffer, buffer_memory, 0);
        return *this;
    }

    vk::CommandBuffer VulkanAPI::begin_single_time_command_buffer()
    {
        vk::CommandBufferAllocateInfo alloc_info(m_command_pool, vk::CommandBufferLevel::ePrimary, 1);
        vk::CommandBuffer command_buffer = m_device.allocateCommandBuffers(alloc_info).front();
        vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        command_buffer.begin(begin_info);
        return command_buffer;
    }

    VulkanAPI& VulkanAPI::end_single_time_command_buffer(const vk::CommandBuffer& command_buffer)
    {
        command_buffer.end();
        vk::SubmitInfo submit_info({}, {}, command_buffer);
        m_graphics_queue.submit(submit_info, {});
        m_graphics_queue.waitIdle();
        m_device.freeCommandBuffers(m_command_pool, command_buffer);
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

    VulkanAPI& VulkanAPI::wait_idle()
    {
        m_device.waitIdle();
        m_graphics_queue.waitIdle();
        m_present_queue.waitIdle();

        return *this;
    }

    VulkanAPI& VulkanAPI::prepare_draw()
    {
        uniform_buffer()->bind();
        m_state->m_pipeline->submit_descriptors();
        return *this;
    }

    VulkanAPI& VulkanAPI::draw(size_t vertex_count, size_t vertices_offset)
    {
        prepare_draw().current_command_buffer().draw(vertex_count, 1, vertices_offset, 0);
        return *this;
    }

    VulkanAPI& VulkanAPI::draw_indexed(size_t indices, size_t offset, size_t vertices_offset)
    {
        prepare_draw().current_command_buffer().drawIndexed(indices, 1, offset, vertices_offset, 0);
        return *this;
    }

    VulkanAPI& VulkanAPI::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
    {
        prepare_draw().current_command_buffer().draw(vertex_count, instances, vertices_offset, 0);
        return *this;
    }

    VulkanAPI& VulkanAPI::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                                 size_t instances)
    {
        prepare_draw().current_command_buffer().drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
        return *this;
    }

    const String& VulkanAPI::renderer()
    {
        return m_renderer;
    }

    const String& VulkanAPI::name()
    {
        static String api_name = "Vulkan";
        return api_name;
    }

    VulkanUniformBuffer* VulkanAPI::uniform_buffer() const
    {
        return m_uniform_buffer[m_current_buffer];
    }

    void VulkanAPI::push_debug_stage(const char* stage, const Color& color)
    {
        if (pfn.vkCmdBeginDebugUtilsLabelEXT)
        {
            VkDebugUtilsLabelEXT label_info = {};
            label_info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label_info.pLabelName           = stage;
            label_info.color[0]             = color.r;
            label_info.color[1]             = color.g;
            label_info.color[2]             = color.b;
            label_info.color[3]             = color.a;

            pfn.vkCmdBeginDebugUtilsLabelEXT(current_command_buffer(), &label_info);
        }
    }

    void VulkanAPI::pop_debug_stage()
    {
        if (pfn.vkCmdEndDebugUtilsLabelEXT)
        {
            pfn.vkCmdEndDebugUtilsLabelEXT(current_command_buffer());
        }
    }
}// namespace Engine
