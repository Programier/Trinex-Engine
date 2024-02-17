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
#include <vulkan_headers.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_swap_chain.hpp>

namespace Engine
{
    struct VulkanTexture;
    struct VulkanViewport;
    struct VulkanUniformBuffer;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete()
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };


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
        static VulkanAPI* _M_vulkan;

        Vector<VulkanExtention> _M_device_extensions;
        Vector<vk::DynamicState> _M_dynamic_states;
        Vector<VulkanUniformBuffer*> _M_uniform_buffer;
        List<Garbage> _M_garbage;


        WindowInterface* _M_window = nullptr;
        String _M_renderer         = "";

        struct {
            PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
            PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT     = nullptr;
        } pfn;


        // API DATA
        struct VulkanState* _M_state = nullptr;
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
        struct VulkanRenderPass* _M_main_render_pass = nullptr;

        vk::CommandPool _M_command_pool;
        uint32_t _M_framebuffers_count = 0;

        size_t _M_current_frame  = 0;
        size_t _M_current_buffer = 0;

        //////////////////////////////////////////////////////////////

        // API METHODS


        vk::SurfaceKHR create_surface(WindowInterface* interface);
        void create_command_pool();
        void create_render_pass(vk::Format);

        void check_extentions();
        void enable_dynamic_states();
        void initialize_pfn();

        vk::Extent2D surface_size() const;
        vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;

        VulkanAPI& create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                                 vk::Buffer& buffer, vk::DeviceMemory& buffer_memory);
        uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

        vk::CommandBuffer begin_single_time_command_buffer();
        VulkanAPI& end_single_time_command_buffer(const vk::CommandBuffer& buffer);

        VulkanAPI& copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size, vk::DeviceSize src_offset = 0,
                               vk::DeviceSize dst_offset = 0);
        bool has_stencil_component(vk::Format format);
        VulkanAPI& create_image(struct VulkanTexture* state, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
                                vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
                                vk::DeviceMemory& image_memory, uint32_t layers);

        vk::CommandBuffer& current_command_buffer();

        //////////////////////////////////////////////////////////////

        VulkanAPI();
        void initialize(WindowInterface* window);
        void initialize_color_formats();

        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& wait_idle() override;


        VulkanAPI& delete_garbage(bool force);
        VulkanAPI& destroy_object(RHI_Object* object) override;
        VulkanAPI& imgui_init(ImGuiContext*) override;
        VulkanAPI& imgui_terminate(ImGuiContext*) override;
        VulkanAPI& imgui_new_frame(ImGuiContext*) override;
        VulkanAPI& imgui_render(ImGuiContext*, ImDrawData* draw_data) override;
        RHI_ImGuiTexture* imgui_create_texture(ImGuiContext*, Texture* texture, Sampler* sampler) override;

        vk::PresentModeKHR present_mode_of(bool vsync);
        bool vsync_from_present_mode(vk::PresentModeKHR);

        const String& renderer() override;
        const String& name() override;

        VulkanAPI& prepare_draw();
        VulkanAPI& draw_indexed(size_t indices, size_t offset) override;
        VulkanAPI& draw(size_t vertex_count) override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data) override;
        RHI_RenderTarget* create_render_target(const RenderTarget*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass(RenderPass* engine_render_pass) override;
        ColorFormatFeatures color_format_features(ColorFormat format) override;
        size_t render_target_buffer_count() override;

        RHI_Viewport* create_viewport(WindowInterface* interface, bool vsync) override;
        RHI_Viewport* create_viewport(RenderTarget* render_target) override;

        VulkanUniformBuffer* uniform_buffer() const;

        VulkanAPI& push_global_params(const GlobalShaderParameters& params) override;
        VulkanAPI& pop_global_params() override;
        VulkanAPI& update_local_parameter(const void* data, size_t size, size_t offset) override;

        ColorFormat base_color_format() override;
        ColorFormat position_format() override;
        ColorFormat normal_format() override;
        ColorFormat specular_format() override;
        ColorFormat depth_format() override;
        ColorFormat stencil_format() override;
        ColorFormat depth_stencil_format() override;

        void push_debug_stage(const char* stage, const Color& color) override;
        void pop_debug_stage() override;

        ~VulkanAPI();
    };

    vk::Format parse_engine_format(ColorFormat format);
    ColorFormat to_engine_format(vk::Format format);
}// namespace Engine
