#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/set.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanBuffer;
	class VulkanTexture;
	class VulkanQueue;
	class VulkanCommandBufferManager;
	class VulkanStaggingBufferManager;
	class VulkanUniformBufferManager;
	class VulkanDescriptorSetAllocator;
	class VulkanStateManager;
	class VulkanQueryPoolManager;
	struct VulkanViewport;
	class Window;

	struct VulkanExtention {
		StringView name;
		mutable bool required = false;
		mutable bool enabled  = false;

		inline bool operator==(const VulkanExtention& ext) const { return name == ext.name; }
		inline bool operator!=(const VulkanExtention& ext) const { return name != ext.name; }

		struct Hasher {
			size_t operator()(const VulkanExtention& ext) const { return std::hash<StringView>()(ext.name); }
		};
	};

	struct VulkanAPI : public RHI {
		trinex_declare_struct(VulkanAPI, void);
		static VulkanAPI* static_constructor();
		static void static_destructor(VulkanAPI* vulkan);

		static VulkanAPI* m_vulkan;

		Set<VulkanExtention, VulkanExtention::Hasher> m_device_extensions;

		struct {
			PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT           = nullptr;
			PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT               = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR = nullptr;
			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR   = nullptr;
			PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT                         = nullptr;

			inline uint32_t getVkHeaderVersion() const { return VK_HEADER_VERSION; }
		} pfn;

		// API DATA
		VulkanStateManager* m_state_manager;
		vkb::Instance m_instance;
		vk::PhysicalDevice m_physical_device;
		vk::Device m_device;
		VmaAllocator m_allocator = VK_NULL_HANDLE;

		VulkanQueue* m_graphics_queue = nullptr;

		vk::PhysicalDeviceProperties m_properties;
		vk::PhysicalDeviceFeatures m_features;

		VulkanCommandBufferManager* m_cmd_manager       = nullptr;
		VulkanStaggingBufferManager* m_stagging_manager = nullptr;

	private:
		MultiMap<uint64_t, class VulkanPipelineLayout*> m_pipeline_layouts;
		VulkanDescriptorSetAllocator* m_descriptor_set_allocator;
		VulkanQueryPoolManager* m_query_pool_manager;


	public:
		VulkanPipelineLayout* create_pipeline_layout(const RHIShaderParameterInfo* parameters, size_t count,
		                                             vk::ShaderStageFlags stages);
		VulkanAPI& destroy_pipeline_layout(VulkanPipelineLayout* layout);

	public:
		inline VulkanDescriptorSetAllocator* descriptor_set_allocator() const { return m_descriptor_set_allocator; }
		inline VulkanCommandBufferManager* command_buffer_mananger() const { return m_cmd_manager; }

	public:
		//////////////////////////////////////////////////////////////

		// API METHODS

		vk::SurfaceKHR create_surface(Window* interface);
		void initialize_pfn();

		vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;
		bool has_stencil_component(vk::Format format);

		class VulkanCommandBuffer* current_command_buffer();

		VulkanCommandBuffer* begin_render_pass();
		VulkanCommandBuffer* end_render_pass();

		bool is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal);
		bool is_extension_enabled(const char* extension);
		//////////////////////////////////////////////////////////////

		VulkanAPI();
		VulkanAPI& initialize(Window* window) override;
		void* context() override;

		VulkanAPI& submit() override;
		VulkanAPI& wait_idle();

		VulkanAPI& bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
		                              RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil) override;
		VulkanAPI& viewport(const RHIViewport& viewport) override;
		VulkanAPI& scissor(const RHIScissors& scissor) override;

		vk::PresentModeKHR present_mode_of(bool vsync, vk::SurfaceKHR surface);

		VulkanAPI& draw(size_t vertex_count, size_t vertices_offset) override;
		VulkanAPI& draw_indexed(size_t indices, size_t offset, size_t vertices_offset) override;
		VulkanAPI& draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances) override;
		VulkanAPI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
		                                  size_t instances) override;

		VulkanAPI& draw_mesh(uint32_t x, uint32_t y, uint32_t z) override;

		VulkanAPI& dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
		VulkanAPI& signal_fence(RHI_Fence* fence) override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHI_Fence* create_fence() override;
		RHI_Sampler* create_sampler(const RHISamplerInitializer*) override;
		RHI_Texture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                            RHITextureCreateFlags flags) override;
		RHI_Shader* create_shader(const byte* shader, size_t size) override;
		RHI_Pipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHI_Pipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHI_Pipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHI_Buffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags) override;
		RHI_Viewport* create_viewport(WindowRenderViewport* viewport, bool vsync) override;
		VulkanAPI& update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;

		VulkanAPI& push_debug_stage(const char* stage) override;
		VulkanAPI& pop_debug_stage() override;

		VulkanAPI& update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data) override;
		VulkanAPI& update_texture(RHI_Texture*, const RHITextureUpdateDesc& desc) override;

		VulkanAPI& copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset,
		                                 size_t dst_offset) override;

		VulkanAPI& copy_texture_to_buffer(RHI_Texture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                  const Vector3u& extent, RHI_Buffer* buffer, size_t buffer_offset) override;

		VulkanAPI& copy_buffer_to_texture(RHI_Buffer* buffer, size_t buffer_offset, RHI_Texture* texture, uint8_t mip_level,
		                                  uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) override;

		VulkanAPI& primitive_topology(RHIPrimitiveTopology topology) override;
		VulkanAPI& polygon_mode(RHIPolygonMode mode) override;
		VulkanAPI& cull_mode(RHICullMode mode) override;
		VulkanAPI& front_face(RHIFrontFace face) override;

		VulkanAPI& bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream) override;
		VulkanAPI& bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format) override;
		VulkanAPI& bind_uniform_buffer(RHI_Buffer* buffer, byte slot) override;
		VulkanAPI& bind_uniform_buffer(VulkanBuffer* buffer, size_t size, size_t offset, byte slot);
		VulkanAPI& bind_srv(RHI_ShaderResourceView* view, byte slot) override;
		VulkanAPI& bind_uav(RHI_UnorderedAccessView* view, byte slot) override;
		VulkanAPI& bind_sampler(RHI_Sampler* view, byte slot) override;

		VulkanAPI& barrier(RHI_Texture* texture, RHIAccess dst_access) override;
		VulkanAPI& barrier(RHI_Buffer* buffer, RHIAccess dst_access) override;

		VulkanAPI& begin_timestamp(RHITimestamp* timestamp) override;
		VulkanAPI& end_timestamp(RHITimestamp* timestamp) override;

		VulkanAPI& begin_statistics(RHIPipelineStatistics* stats) override;
		VulkanAPI& end_statistics(RHIPipelineStatistics* stats) override;

		~VulkanAPI();
	};
}// namespace Engine
