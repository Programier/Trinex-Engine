#pragma once
#include <Core/logger.hpp>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <VkBootstrap.h>
#include <api.hpp>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <vulkan_api.hpp>
#include <vulkan_framebuffer.hpp>
#include <vulkan_swap_chain.hpp>

#define MAIN_FRAMEBUFFERS_COUNT 3
#define API VulkanAPI::_M_vulkan
#define VIEW_PORT API->window_data.view_port

#define ENABLE_VALIDATION_LAYERS 1

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


    using ViewPort = vk::Viewport;


    struct VulkanAPI : public GraphicApiInterface::ApiInterface {
        static std::vector<const char*> device_extensions;

        static VulkanAPI* _M_vulkan;
        Logger* _M_current_logger = nullptr;
        SDL_Window* _M_window = nullptr;
        bool _M_need_recreate_swap_chain = false;

        struct {
            int width, height;
            ViewPort view_port;
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

        SwapChain* _M_swap_chain = nullptr;

        vk::RenderPass _M_render_pass;
        std::vector<VulkanFramebuffer*> _M_swap_chain_framebuffers;
        VulkanFramebuffer* _M_current_framebuffer = nullptr;
        vk::CommandPool _M_command_pool;
        std::vector<vk::CommandBuffer> _M_command_buffers;
        vk::CommandBuffer* _M_current_command_buffer = nullptr;
        vk::ResultValue<uint32_t> _M_current_buffer_index;
        vk::PresentModeKHR _M_swap_chain_mode = vk::PresentModeKHR::eFifo;

        std::vector<vk::Semaphore> _M_image_available_semaphores;
        std::vector<vk::Semaphore> _M_render_finished_semaphores;
        std::vector<vk::Fence> _M_in_flight_fences;

        uint32_t _M_current_frame = 1;
        vk::ClearValue _M_clear_values[3];
        float _M_min_depth = 0.f;
        float _M_max_depth = 1.f;


        //////////////////////////////////////////////////////////////

        // API METHODS


        void init();
        void create_surface();

        void destroy_framebuffers();
        void recreate_swap_chain();
        void create_swap_chain();
        void create_render_pass();
        void create_framebuffers();
        void create_command_buffer();
        void create_semaphores();

        VulkanAPI& create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
                                 vk::Buffer& buffer, vk::DeviceMemory& buffer_memory);
        uint32_t find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties);

        //////////////////////////////////////////////////////////////

        VulkanAPI();
        void* init_window(SDL_Window* window) override;
        VulkanAPI& destroy_window() override;
        VulkanAPI& on_window_size_changed() override;
        VulkanAPI& swap_buffer(SDL_Window* window) override;
        VulkanAPI& begin_render() override;
        VulkanAPI& end_render() override;
        VulkanAPI& begin_render_pass() override;
        VulkanAPI& end_render_pass() override;
        VulkanAPI& framebuffer_viewport(const Point2D& point, const Size2D& size) override;
        VulkanAPI& bind_framebuffer(const ObjID& ID) override;
        VulkanAPI& clear_color(const Color& color) override;
        VulkanAPI& clear_frame_buffer(const ObjID&, BufferType) override;
        VulkanAPI& swap_interval(int_t interval) override;
        VulkanAPI& wait_idle() override;

        VulkanAPI& destroy_object(ObjID& ID) override;

        VulkanAPI& create_shader(ObjID&, const ShaderParams&) override;
        VulkanAPI& use_shader(const ObjID&) override;
        //ApiInterface& shader_value(const ObjID&, const std::string&, float) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, int_t) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, const glm::mat3&) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, const glm::mat4&) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, const glm::vec2&) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, const glm::vec3&) VIRTUAL_METHOD;
        //ApiInterface& shader_value(const ObjID&, const std::string&, const glm::vec4&) VIRTUAL_METHOD;
        VulkanAPI& shader_value(const ObjID& ID, const String& name, void* data);

        ~VulkanAPI();
    };
}// namespace Engine
