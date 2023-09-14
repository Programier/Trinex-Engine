/*
    The code of implementation the API does need much data hiding,
    as this code will only be used by the engine, not the end user
*/

#pragma once
#include <Core/logger.hpp>
#include <VkBootstrap.h>
#include <api.hpp>
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

    struct VulkanAPI : public GraphicApiInterface::ApiInterface {
        static Vector<const char*> device_extensions;
        static VulkanAPI* _M_vulkan;
        Logger** _M_engine_logger        = nullptr;
        WindowInterface* _M_window            = nullptr;
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
        void* init_window(WindowInterface* window) override;
        VulkanAPI& destroy_window() override;
        VulkanAPI& on_window_size_changed() override;
        VulkanAPI& swap_buffer() override;
        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& framebuffer_viewport(const Identifier&, const ViewPort& viewport) override;
        VulkanAPI& bind_framebuffer(const Identifier& ID, size_t buffer_index) override;
        VulkanAPI& clear_color(const Identifier& ID, const ColorClearValue& color, byte layout) override;
        VulkanAPI& swap_interval(int_t interval) override;
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

        VulkanAPI& create_texture(Identifier& ID, const TextureCreateInfo& info, TextureType type) override;
        VulkanAPI& bind_texture(const Identifier&, TextureBindIndex binding) override;

        MipMapLevel base_level_texture(const Identifier& ID) override;
        VulkanAPI& base_level_texture(const Identifier& ID, MipMapLevel level) override;
        CompareFunc compare_func_texture(const Identifier& ID) override;
        VulkanAPI& compare_func_texture(const Identifier& ID, CompareFunc func) override;
        CompareMode compare_mode_texture(const Identifier& ID) override;
        VulkanAPI& compare_mode_texture(const Identifier&, CompareMode mode) override;
        TextureFilter min_filter_texture(const Identifier& ID) override;
        TextureFilter mag_filter_texture(const Identifier& ID) override;
        VulkanAPI& min_filter_texture(const Identifier& ID, TextureFilter filter) override;
        VulkanAPI& mag_filter_texture(const Identifier& ID, TextureFilter filter) override;
        VulkanAPI& min_lod_level_texture(const Identifier&, LodLevel) override;
        VulkanAPI& max_lod_level_texture(const Identifier&, LodLevel) override;
        LodLevel min_lod_level_texture(const Identifier& ID) override;
        LodLevel max_lod_level_texture(const Identifier& ID) override;
        MipMapLevel max_mipmap_level_texture(const Identifier& ID) override;
        VulkanAPI& swizzle_texture(const Identifier& ID, const SwizzleRGBA& swizzle) override;
        SwizzleRGBA swizzle_texture(const Identifier& ID) override;
        VulkanAPI& wrap_s_texture(const Identifier& ID, const WrapValue& value) override;
        VulkanAPI& wrap_t_texture(const Identifier& ID, const WrapValue& value) override;
        VulkanAPI& wrap_r_texture(const Identifier& ID, const WrapValue& value) override;
        WrapValue wrap_s_texture(const Identifier&) override;
        WrapValue wrap_t_texture(const Identifier&) override;
        WrapValue wrap_r_texture(const Identifier&) override;
        VulkanAPI& logger(Logger*&) override;
        VulkanAPI& generate_texture_mipmap(const Identifier&) override;
        VulkanAPI& update_texture_2D(const Identifier&, const Size2D&, const Offset2D&, MipMapLevel,
                                     const void*) override;
        VulkanAPI& texture_size(const Identifier& ID, Size2D& size, MipMapLevel level) override;

        VulkanAPI& read_texture_2D_data(const Identifier&, Vector<byte>& data, MipMapLevel) override;
        VulkanAPI& anisotropic_filtering_texture(const Identifier& ID, float value) override;
        float anisotropic_filtering_texture(const Identifier& ID) override;
        float max_anisotropic_filtering() override;


        VulkanAPI& cubemap_texture_update_data(const Identifier&, TextureCubeMapFace, const Size2D&, const Offset2D&,
                                               MipMapLevel, void*) override;

        SamplerMipmapMode sample_mipmap_mode_texture(const Identifier& ID) override;
        VulkanAPI& sample_mipmap_mode_texture(const Identifier& ID, SamplerMipmapMode mode) override;
        LodBias lod_bias_texture(const Identifier& ID) override;
        VulkanAPI& lod_bias_texture(const Identifier& ID, LodBias bias) override;
        LodBias max_lod_bias_texture() override;

        VulkanAPI& create_ssbo(Identifier&, const byte* data, size_t size) override;
        VulkanAPI& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        VulkanAPI& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        VulkanAPI& create_uniform_buffer(Identifier&, const byte*, size_t) override;
        VulkanAPI& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        VulkanAPI& bind_uniform_buffer(const Identifier&, BindingIndex binding) override;
        MappedMemory map_uniform_buffer(const Identifier& ID) override;
        VulkanAPI& unmap_uniform_buffer(const Identifier& ID) override;

        VulkanAPI& gen_framebuffer(Identifier&, const FrameBufferCreateInfo& info) override;
        Identifier imgui_texture_id(const Identifier&) override;
        VulkanAPI& framebuffer_scissor(const Identifier&, const Scissor&) override;
        VulkanAPI& clear_depth_stencil(const Identifier&, const DepthStencilClearValue&) override;
        bool check_format_support(PixelType type, PixelComponentType component) override;

        VulkanAPI& async_render(bool flag) override;
        bool async_render() override;
        VulkanAPI& next_render_thread() override;
        ~VulkanAPI();
    };
}// namespace Engine
