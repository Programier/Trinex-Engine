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
#include <vulkan_state.hpp>
#include <vulkan_swap_chain.hpp>

namespace Engine
{
    struct VulkanTexture;
    struct VulkanViewport;
    struct VulkanUniformBuffer;
    class Window;

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

        Window* m_window = nullptr;

        struct {
            PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
            PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT     = nullptr;
        } pfn;


        // API DATA
        VulkanState m_state;
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

        vk::CommandPool m_command_pool;
        uint32_t m_framebuffers_count = 0;

        size_t m_current_frame  = 0;
        size_t m_current_buffer = 0;

        //////////////////////////////////////////////////////////////

        // API METHODS


        vk::SurfaceKHR create_surface(Window* interface);
        void create_command_pool();
        void enable_dynamic_states();
        void initialize_pfn();

        VulkanViewportMode find_current_viewport_mode();
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

        struct VulkanCommandBuffer* current_command_buffer();
        vk::CommandBuffer& current_command_buffer_handle();

        //////////////////////////////////////////////////////////////

        VulkanAPI();
        VulkanAPI& initialize(Window* window) override;
        void* context() override;

        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& wait_idle();

        VulkanAPI& bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) override;
        VulkanAPI& viewport(const ViewPort& viewport) override;
        ViewPort viewport() override;
        VulkanAPI& scissor(const Scissor& scissor) override;
        Scissor scissor() override;

        vk::PresentModeKHR present_mode_of(bool vsync);
        bool vsync_from_present_mode(vk::PresentModeKHR);

        VulkanAPI& prepare_draw();
        VulkanAPI& draw(size_t vertex_count, size_t vertices_offset) override;
        VulkanAPI& draw_indexed(size_t indices, size_t offset, size_t vertices_offset) override;
        VulkanAPI& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
        VulkanAPI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                          size_t instances) override;

        RHI_Sampler* create_sampler(const Sampler*) override;
        RHI_Texture* create_texture_2d(const Texture2D*) override;
        RHI_Texture* create_render_surface(const RenderSurface*) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
        RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
        RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferFormat format, RHIBufferType type) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
        RHI_Viewport* create_viewport(RenderViewport* viewport) override;

        VulkanUniformBuffer* uniform_buffer() const;

        VulkanAPI& push_global_params(const GlobalShaderParameters& params) override;
        VulkanAPI& pop_global_params() override;
        VulkanAPI& update_local_parameter(const void* data, size_t size, size_t offset) override;

        VulkanAPI& push_debug_stage(const char* stage, const Color& color) override;
        VulkanAPI& pop_debug_stage() override;

        ~VulkanAPI();
    };

    constexpr inline vk::Format parse_engine_format(ColorFormat format)
    {
        switch (format)
        {
            case ColorFormat::Undefined:
                return vk::Format::eUndefined;
            case ColorFormat::FloatR:
                return vk::Format::eR16Sfloat;
            case ColorFormat::FloatRGBA:
                return vk::Format::eR16G16B16A16Sfloat;
            case ColorFormat::R8:
                return vk::Format::eR8Unorm;
            case ColorFormat::R8G8B8A8:
                return vk::Format::eR8G8B8A8Unorm;
            case ColorFormat::DepthStencil:
                return vk::Format::eD24UnormS8Uint;
            case ColorFormat::ShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::FilteredShadowDepth:
                return vk::Format::eD32Sfloat;
            case ColorFormat::D32F:
                return vk::Format::eD32Sfloat;
            case ColorFormat::BC1:
                return vk::Format::eBc1RgbaUnormBlock;
            case ColorFormat::BC2:
                return vk::Format::eBc2UnormBlock;
            case ColorFormat::BC3:
                return vk::Format::eBc3UnormBlock;

            default:
                return vk::Format::eUndefined;
        }
    }

    constexpr inline ColorFormat to_engine_format(vk::Format format)
    {
        switch (format)
        {
            case vk::Format::eUndefined:
                return ColorFormat::Undefined;
            case vk::Format::eR16Sfloat:
                return ColorFormat::FloatR;
            case vk::Format::eR16G16B16A16Sfloat:
                return ColorFormat::FloatRGBA;
            case vk::Format::eR8Unorm:
                return ColorFormat::R8;
            case vk::Format::eR8G8B8A8Unorm:
                return ColorFormat::R8G8B8A8;
            case vk::Format::eD24UnormS8Uint:
                return ColorFormat::DepthStencil;
            case vk::Format::eD32Sfloat:
                return ColorFormat::ShadowDepth;
            case vk::Format::eBc1RgbaUnormBlock:
                return ColorFormat::BC1;
            case vk::Format::eBc2UnormBlock:
                return ColorFormat::BC2;
            case vk::Format::eBc3UnormBlock:
                return ColorFormat::BC3;
            default:
                return ColorFormat::Undefined;
        }

        return ColorFormat::Undefined;
    }
}// namespace Engine
