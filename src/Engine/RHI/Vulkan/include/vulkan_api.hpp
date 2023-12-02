/*
    The code of implementation the API does need much data hiding,
    as this code will only be used by the engine, not the end user
*/

#pragma once
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <optional>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_headers.hpp>
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

    struct Garbage {
        RHI_Object* object;
        size_t frame;

        Garbage(RHI_Object* object, size_t frame) : object(object), frame(frame)
        {}
    };


    struct VulkanExtention {
        const char* name;
        bool required = false;
        bool enabled  = false;
    };

    struct VulkanAPI : public RHI {
        Vector<VulkanExtention> _M_device_extensions;
        Vector<vk::DynamicState> _M_dynamic_states;

        static VulkanAPI* _M_vulkan;

        List<Garbage> _M_garbage;

        WindowInterface* _M_window       = nullptr;
        bool _M_need_recreate_swap_chain = false;
        String _M_renderer               = "";

        struct {
            int width, height;
        } window_data;

        struct {
            PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
            PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT     = nullptr;
        } pfn;


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
        vk::PhysicalDeviceFeatures _M_features;

        vk::SurfaceCapabilitiesKHR _M_surface_capabilities;

        vk::DescriptorPool _M_imgui_descriptor_pool;
        SwapChain* _M_swap_chain = nullptr;

        VulkanMainFrameBuffer* _M_main_framebuffer   = nullptr;
        struct VulkanRenderPass* _M_main_render_pass = nullptr;
        struct VulkanState* _M_state                 = nullptr;

        vk::CommandPool _M_command_pool;
        vk::PresentModeKHR _M_swap_chain_mode;

        uint32_t _M_framebuffers_count = 0;
        uint32_t _M_image_index        = 0;
        uint32_t _M_current_buffer     = 0;
        uint32_t _M_prev_buffer        = 0;
        size_t _M_current_frame        = 0;

        //////////////////////////////////////////////////////////////

        // API METHODS


        void init();
        void create_surface();
        void destroy_framebuffers(bool full_destroy = true);
        void recreate_swap_chain();
        void create_swap_chain();
        void create_framebuffers();
        void create_command_buffer();
        void create_render_pass();

        void check_extentions();
        void enable_dynamic_states();
        void initialize_pfn();


        vk::Extent2D surface_size() const;

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
        VulkanAPI& create_image(struct VulkanTexture* state, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                                vk::DeviceMemory& image_memory, uint32_t layers);

        vk::CommandBuffer& current_command_buffer();
        struct VulkanMainRenderTargetFrame* current_main_frame();

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


        VulkanAPI& delete_garbage(bool force);
        VulkanAPI& destroy_object(RHI_Object* object) override;
        VulkanAPI& imgui_init() override;
        VulkanAPI& imgui_terminate() override;
        VulkanAPI& imgui_new_frame() override;
        VulkanAPI& imgui_render() override;

        String renderer() override;

        VulkanAPI& draw_indexed(size_t indices, size_t offset) override;
        VulkanAPI& draw(size_t vertex_count) override;

        Identifier imgui_texture_id(const Identifier&) override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data) override;
        RHI_RenderTarget* window_render_target() override;
        RHI_RenderTarget* create_render_target(const RenderTarget*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass() override;
        ColorFormatFeatures color_format_features(ColorFormat format) override;
        size_t render_target_buffer_count() override;

        void line_width(float width) override;

        void push_debug_stage(const char* stage, const Color& color) override;
        void pop_debug_stage() override;

        ~VulkanAPI();
    };
}// namespace Engine
