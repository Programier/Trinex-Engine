/*
    The code of implementation the API does need much data hiding,
    as this code will only be used by the engine, not the end user
*/

#pragma once
#include <Core/etl/vector.hpp>
#include <Core/logger.hpp>
#include <Graphics/rhi.hpp>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	struct VulkanTexture;
	struct VulkanViewport;
	class VulkanUniformBufferManager;
	class Window;

	struct VulkanExtention {
		const char* name;
		bool required = false;
		bool enabled  = false;
	};

	static constexpr inline vk::PipelineStageFlags all_shaders_stage =
	        vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eTessellationControlShader |
	        vk::PipelineStageFlagBits::eTessellationEvaluationShader | vk::PipelineStageFlagBits::eGeometryShader |
	        vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eComputeShader;

	struct VulkanAPI : public RHI {
		trinex_declare_struct(VulkanAPI, void);
		static VulkanAPI* static_constructor();
		static void static_destructor(VulkanAPI* vulkan);

		static VulkanAPI* m_vulkan;

		Vector<VulkanExtention> m_device_extensions;

		struct {
			PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT           = nullptr;
			PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT               = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR = nullptr;
			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR   = nullptr;
		} pfn;

		// API DATA
		VulkanState m_state;
		vkb::Instance m_instance;
		vk::PhysicalDevice m_physical_device;
		vk::Device m_device;
		VmaAllocator m_allocator = VK_NULL_HANDLE;

		struct VulkanQueue* m_graphics_queue = nullptr;
		struct VulkanQueue* m_present_queue  = nullptr;

		vk::PhysicalDeviceProperties m_properties;
		vk::PhysicalDeviceFeatures m_features;

		struct VulkanCommandBufferManager* m_cmd_manager       = nullptr;
		struct VulkanStaggingBufferManager* m_stagging_manager = nullptr;

		//////////////////////////////////////////////////////////////

		// API METHODS

		vk::SurfaceKHR create_surface(Window* interface);
		VulkanAPI& setup_present_queue(vk::SurfaceKHR surface);
		void initialize_pfn();

		VulkanViewportMode find_current_viewport_mode();
		vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;
		bool has_stencil_component(vk::Format format);

		struct VulkanCommandBuffer* current_command_buffer();
		vk::CommandBuffer& current_command_buffer_handle();
		VulkanUniformBufferManager* uniform_buffer_manager();

		VulkanCommandBuffer* begin_render_pass();
		VulkanCommandBuffer* end_render_pass();

		//////////////////////////////////////////////////////////////

		VulkanAPI();
		VulkanAPI& initialize(Window* window) override;
		void* context() override;

		VulkanAPI& submit() override;
		VulkanAPI& wait_idle();

		VulkanAPI& bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                              RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil) override;
		VulkanAPI& viewport(const ViewPort& viewport) override;
		ViewPort viewport() override;
		VulkanAPI& scissor(const Scissor& scissor) override;
		Scissor scissor() override;

		vk::PresentModeKHR present_mode_of(bool vsync, vk::SurfaceKHR surface);

		VulkanAPI& prepare_draw();
		VulkanAPI& prepare_dispatch();
		VulkanAPI& draw(size_t vertex_count, size_t vertices_offset) override;
		VulkanAPI& draw_indexed(size_t indices, size_t offset, size_t vertices_offset) override;
		VulkanAPI& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
		VulkanAPI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                  size_t instances) override;

		VulkanAPI& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;

		RHI_Sampler* create_sampler(const Sampler*) override;
		RHI_Texture2D* create_texture_2d(ColorFormat format, Vector2u size, uint32_t mips, TextureCreateFlags flags) override;
		RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
		RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader) override;
		RHI_Shader* create_tesselation_shader(const TessellationShader* shader) override;
		RHI_Shader* create_geometry_shader(const GeometryShader* shader) override;
		RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
		RHI_Shader* create_compute_shader(const ComputeShader* shader) override;
		RHI_Pipeline* create_graphics_pipeline(const GraphicsPipeline* pipeline) override;
		RHI_Pipeline* create_compute_pipeline(const ComputePipeline* pipeline) override;
		RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, RHIIndexFormat format, RHIBufferType type) override;
		RHI_SSBO* create_ssbo(size_t size, const byte* data, RHIBufferType type) override;
		RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data, RHIBufferType type) override;
		RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;
		VulkanAPI& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;

		VulkanAPI& push_debug_stage(const char* stage, const LinearColor& color) override;
		VulkanAPI& pop_debug_stage() override;

		~VulkanAPI();
	};
}// namespace Engine
