/*
    The code of implementation the API does need much data hiding,
    as this code will only be used by the engine, not the end user
*/

#pragma once
#include <Core/logger.hpp>
#include <VkBootstrap.h>
#include <Graphics/rhi.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <vulkan_api.hpp>
#include <vulkan_block_allocator.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_swap_chain.hpp>

namespace Engine
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete()
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };


    struct VulkanTexture;

    struct VulkanAPI : public RHI {
        static Vector<const char*> device_extensions;
        static VulkanAPI* _M_vulkan;
        WindowInterface* _M_window       = nullptr;
        bool _M_need_recreate_swap_chain = false;
        String _M_renderer               = "";

        struct {
            int width, height;
        } window_data;

        // API DATA
        vkb::Instance _M_instance;
        vk::SurfaceKHR _M_surface;
        vk::PhysicalDevice _M_physical_device;
        vk::Device _M_device;
        vkb::Device _M_bootstrap_device;
        QueueFamilyIndices _M_graphics_and_present_index;
        vk::Queue _M_graphics_queue;
        vk::Queue _M_present_queue;

        vk::PhysicalDeviceProperties _M_properties;
        BlockAllocator<struct VulkanUniformBufferBlock*, UNIFORM_BLOCK_SIZE> _M_uniform_allocator;
        vk::DescriptorPool _M_imgui_descriptor_pool;

        bool _M_need_update_image_index = true;
        SwapChain* _M_swap_chain        = nullptr;

        VulkanFramebuffer* _M_main_framebuffer = nullptr;
        struct VulkanState* _M_state           = nullptr;

        vk::CommandPool _M_command_pool;
        struct VulkanCommandBuffer* _M_command_buffer = nullptr;
        vk::PresentModeKHR _M_swap_chain_mode;

        uint32_t _M_current_buffer = 0;
        uint32_t _M_current_frame  = 0;

        //////////////////////////////////////////////////////////////

        // API METHODS


        void init();
        void create_surface();
        void destroy_framebuffers(bool full_destroy = true);
        void recreate_swap_chain();
        void create_swap_chain();
        VulkanFramebuffer* init_base_framebuffer_renderpass(VulkanFramebuffer* framebuffer);
        void create_framebuffers();
        void create_command_buffer();

        VulkanAPI& create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                                 vk::Buffer& buffer, vk::DeviceMemory& buffer_memory);
        uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

        vk::CommandBuffer begin_single_time_command_buffer();
        VulkanAPI& end_single_time_command_buffer(const vk::CommandBuffer& buffer);

        VulkanAPI& copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size,
                               vk::DeviceSize src_offset = 0, vk::DeviceSize dst_offset = 0);
        vk::ResultValue<uint32_t> swapchain_image_index();
        vk::Format find_supported_format(const Vector<vk::Format>& candidates, vk::ImageTiling tiling,
                                         vk::FormatFeatureFlags features);
        bool has_stencil_component(vk::Format format);
        vk::Format find_depth_format();
        VulkanAPI& create_resolve_image(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                                        vk::ImageCreateFlags flags, vk::ImageUsageFlags usage,
                                        vk::MemoryPropertyFlags properties, vk::Image& image,
                                        vk::DeviceMemory& image_memory, uint32_t mip_count = 1, uint32_t layers = 1);

        VulkanAPI& create_image(struct VulkanTexture* state, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                                vk::DeviceMemory& image_memory, uint32_t layers);

        VulkanFramebuffer* framebuffer(Identifier ID);
        //////////////////////////////////////////////////////////////

        VulkanAPI();
        void* init_window(WindowInterface* window, const WindowConfig& config) override;
        VulkanAPI& destroy_window() override;
        VulkanAPI& on_window_size_changed() override;
        VulkanAPI& swap_buffer() override;
        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& vsync(bool) override;
        bool vsync() override;
        VulkanAPI& wait_idle() override;

        VulkanAPI& imgui_init() override;
        VulkanAPI& imgui_terminate() override;
        VulkanAPI& imgui_new_frame() override;
        VulkanAPI& imgui_render() override;

        String renderer() override;
        VulkanAPI& destroy_object(Identifier& ID) override;

        VulkanAPI& create_shader(Identifier&, const PipelineCreateInfo&) override;
        VulkanAPI& use_shader(const Identifier&) override;

        VulkanAPI& create_vertex_buffer(Identifier&, const byte*, size_t) override;
        VulkanAPI& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        VulkanAPI& bind_vertex_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_vertex_buffer(const Identifier& ID) override;
        VulkanAPI& unmap_vertex_buffer(const Identifier& ID) override;

        VulkanAPI& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) override;
        VulkanAPI& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        VulkanAPI& bind_index_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_index_buffer(const Identifier& ID) override;
        VulkanAPI& unmap_index_buffer(const Identifier& ID) override;
        VulkanAPI& draw_indexed(size_t indices, size_t offset) override;

        VulkanAPI& create_ssbo(Identifier&, const byte* data, size_t size) override;
        VulkanAPI& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        VulkanAPI& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        VulkanAPI& create_uniform_buffer(Identifier&, const byte*, size_t) override;
        VulkanAPI& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        VulkanAPI& bind_uniform_buffer(const Identifier&, BindingIndex binding) override;
        MappedMemory map_uniform_buffer(const Identifier& ID) override;
        VulkanAPI& unmap_uniform_buffer(const Identifier& ID) override;

        Identifier imgui_texture_id(const Identifier&) override;
        bool check_format_support(ColorFormat format) override;

        VulkanAPI& async_render(bool flag) override;
        bool async_render() override;
        VulkanAPI& next_render_thread() override;


        RHI_Sampler* create_sampler(const SamplerCreateInfo&) override;
        RHI_Texture* create_texture(const TextureCreateInfo&, TextureType, const byte* data) override;
        RHI_FrameBuffer* window_framebuffer() override;
        RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info) override;
        ~VulkanAPI();
    };
}// namespace Engine
