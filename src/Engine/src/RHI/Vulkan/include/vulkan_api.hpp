/*
    The code of implementation the API does need much data hiding,
    as this code will only be used by the engine, not the end user
*/

#pragma once
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
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
        static VulkanAPI* m_vulkan;

        Vector<VulkanExtention> m_device_extensions;
        Vector<vk::DynamicState> m_dynamic_states;
        Vector<VulkanUniformBuffer*> m_uniform_buffer;
        List<Garbage> m_garbage;


        WindowInterface* m_window = nullptr;
        String m_renderer         = "";

        struct {
            PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
            PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT     = nullptr;
        } pfn;


        // API DATA
        struct VulkanState* m_state = nullptr;
        vkb::Instance m_instance;
        vk::SurfaceKHR m_surface;
        vk::PhysicalDevice m_physical_device;
        vk::Device m_device;
        vkb::Device m_bootstrap_device;

        uint32_t m_graphics_queue_index;
        uint32_t m_present_queue_index;
        vk::Queue m_graphics_queue;
        vk::Queue m_present_queue;

        vk::PhysicalDeviceProperties m_properties;
        vk::PhysicalDeviceFeatures m_features;
        vk::SurfaceCapabilitiesKHR m_surface_capabilities;


        vk::DescriptorPool m_imgui_descriptor_pool;
        struct VulkanRenderPass* m_main_render_pass = nullptr;

        vk::CommandPool m_command_pool;
        uint32_t m_framebuffers_count = 0;

        size_t m_current_frame  = 0;
        size_t m_current_buffer = 0;

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

        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& wait_idle() override;


        VulkanAPI& delete_garbage(bool force);
        VulkanAPI& destroy_object(RHI_Object* object) override;
        VulkanAPI& imgui_init(ImGuiContext*) override;
        VulkanAPI& imgui_terminate(ImGuiContext*) override;
        VulkanAPI& imgui_new_frame(ImGuiContext*) override;
        VulkanAPI& imgui_render(ImGuiContext*, ImDrawData* draw_data) override;

        vk::PresentModeKHR present_mode_of(bool vsync);
        bool vsync_from_present_mode(vk::PresentModeKHR);

        const String& renderer() override;
        const String& name() override;

        VulkanAPI& prepare_draw();
        VulkanAPI& draw(size_t vertex_count, size_t vertices_offset) override;
        VulkanAPI& draw_indexed(size_t indices, size_t offset, size_t vertices_offset) override;
        VulkanAPI& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
        VulkanAPI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                          size_t instances) override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture(const Texture*, const byte* data, size_t size) override;
        RHI_RenderTarget* create_render_target(const RenderTarget*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass(RenderPass* engine_render_pass) override;

        RHI_Viewport* create_viewport(WindowInterface* interface, bool vsync) override;
        RHI_Viewport* create_viewport(RenderTarget* render_target) override;

        VulkanUniformBuffer* uniform_buffer() const;

        VulkanAPI& push_global_params(const GlobalShaderParameters& params) override;
        VulkanAPI& pop_global_params() override;
        VulkanAPI& update_local_parameter(const void* data, size_t size, size_t offset) override;

        void push_debug_stage(const char* stage, const Color& color) override;
        void pop_debug_stage() override;

        ~VulkanAPI();
    };

    vk::Format parse_engine_format(ColorFormat format);
}// namespace Engine