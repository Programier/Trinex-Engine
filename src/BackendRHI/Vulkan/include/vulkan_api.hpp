#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/atomic.hpp>
#include <Core/etl/critical_section.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/map.hpp>
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
	class VulkanDescriptorHeap;
	class Window;

	struct VulkanExtention {
		StringView name;
		mutable bool enabled = false;

		inline bool operator==(const VulkanExtention& ext) const { return name == ext.name; }
		inline bool operator!=(const VulkanExtention& ext) const { return name != ext.name; }
		inline bool is_valid() const { return !name.empty(); }
	};

	class VulkanAPI : public RHI
	{
		trinex_declare_struct(VulkanAPI, void);

	public:
		static VulkanAPI* static_constructor();
		static void static_destructor(VulkanAPI* vulkan);

		static VulkanAPI* m_vulkan;

		struct {
			PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT           = nullptr;
			PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT               = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR = nullptr;
			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR   = nullptr;
			PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT                         = nullptr;
			PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR         = nullptr;

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
		struct VulkanUpdater;

		struct Garbage {
			RHIObject* object;
			uint64_t frame;
		};

		CriticalSection m_cs;
		Atomic<uint64_t> m_frame;

		Deque<Garbage> m_garbage;
		Vector<VulkanExtention> m_device_extensions;
		MultiMap<uint64_t, class VulkanPipelineLayout*> m_pipeline_layouts;
		VulkanDescriptorSetAllocator* m_descriptor_set_allocator;
		VulkanQueryPoolManager* m_query_pool_manager;
		VulkanDescriptorHeap* m_descriptor_heap;
		VulkanUpdater* m_updater = nullptr;

		vk::Semaphore m_timeline;

	private:
		static consteval auto make_extensions_array()
		{
			return std::to_array<VulkanExtention>({
			        {"", false},// Dummy extension
			        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
			        {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, true},
			        {VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, true},
			        {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false},
			        {VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, false},
			        {VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, false},
			        {VK_KHR_SPIRV_1_4_EXTENSION_NAME, false},
			        {VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, false},
			        {VK_EXT_MESH_SHADER_EXTENSION_NAME, false},
			});
		}

		VulkanAPI& update(float dt);
		VulkanAPI& destroy_garbage();

	public:
		VulkanPipelineLayout* create_pipeline_layout(const RHIShaderParameterInfo* parameters, size_t count,
		                                             vk::ShaderStageFlags stages);
		VulkanAPI& destroy_pipeline_layout(VulkanPipelineLayout* layout);

		static consteval size_t find_extension_index(const char* str)
		{
			auto extensions = make_extensions_array();

			for (size_t i = 1, count = extensions.size(); i < count; ++i)
			{
				if (extensions[i].name == str)
					return i;
			}

			return 0;
		}

	public:
		inline const Vector<VulkanExtention>& extensions() const { return m_device_extensions; }
		inline bool is_extension_enabled(size_t index) const { return m_device_extensions[index].enabled; }
		inline VulkanDescriptorSetAllocator* descriptor_set_allocator() const { return m_descriptor_set_allocator; }
		inline VulkanCommandBufferManager* command_buffer_mananger() const { return m_cmd_manager; }
		inline VulkanDescriptorHeap* descriptor_heap() const { return m_descriptor_heap; }

	public:
		//////////////////////////////////////////////////////////////

		// API METHODS

		vk::SurfaceKHR create_surface(Window* interface);
		void initialize_pfn();

		vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;
		bool has_stencil_component(vk::Format format);

		class VulkanCommandHandle* current_command_buffer();

		VulkanCommandHandle* begin_render_pass();
		VulkanCommandHandle* end_render_pass();

		bool is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal);
		VulkanAPI& add_garbage(RHIObject* object);
		//////////////////////////////////////////////////////////////

		VulkanAPI();

		VulkanAPI& submit() override;
		VulkanAPI& wait_idle();

		VulkanAPI& bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
		                              RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil) override;
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
		VulkanAPI& signal_fence(RHIFence* fence) override;

		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence() override;
		RHISampler* create_sampler(const RHISamplerInitializer*) override;
		RHITexture* create_texture(RHITextureType type, RHIColorFormat format, Vector3u size, uint32_t mips,
		                           RHITextureCreateFlags flags) override;
		RHIShader* create_shader(const byte* shader, size_t size) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineInitializer* pipeline) override;
		RHIBuffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context() override;
		VulkanAPI& update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index) override;

		VulkanAPI& push_debug_stage(const char* stage) override;
		VulkanAPI& pop_debug_stage() override;

		VulkanAPI& update_buffer(RHIBuffer* buffer, size_t offset, size_t size, const byte* data) override;
		VulkanAPI& update_texture(RHITexture* texture, const RHITextureRegion& region, const void* data, size_t size,
		                          size_t buffer_width, size_t buffer_height) override;

		VulkanAPI& copy_buffer_to_buffer(RHIBuffer* src, RHIBuffer* dst, size_t size, size_t src_offset,
		                                 size_t dst_offset) override;

		VulkanAPI& copy_texture_to_buffer(RHITexture* texture, uint8_t mip_level, uint16_t array_slice, const Vector3u& offset,
		                                  const Vector3u& extent, RHIBuffer* buffer, size_t buffer_offset) override;

		VulkanAPI& copy_buffer_to_texture(RHIBuffer* buffer, size_t buffer_offset, RHITexture* texture, uint8_t mip_level,
		                                  uint16_t array_slice, const Vector3u& offset, const Vector3u& extent) override;

		VulkanAPI& copy_texture_to_texture(RHITexture* src, const RHITextureRegion& src_region, RHITexture* dst,
		                                   const RHITextureRegion& dst_region) override;

		VulkanAPI& primitive_topology(RHIPrimitiveTopology topology) override;
		VulkanAPI& polygon_mode(RHIPolygonMode mode) override;
		VulkanAPI& cull_mode(RHICullMode mode) override;
		VulkanAPI& front_face(RHIFrontFace face) override;
		VulkanAPI& write_mask(RHIColorComponent mask) override;

		VulkanAPI& bind_vertex_attribute(RHIVertexSemantic semantic, byte semantic_index, byte stream, uint16_t offset) override;
		VulkanAPI& bind_vertex_buffer(RHIBuffer* buffer, size_t byte_offset, uint16_t stride, byte stream,
		                              RHIVertexInputRate rate) override;
		VulkanAPI& bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format) override;
		VulkanAPI& bind_uniform_buffer(RHIBuffer* buffer, byte slot) override;
		VulkanAPI& bind_uniform_buffer(VulkanBuffer* buffer, size_t size, size_t offset, byte slot);
		VulkanAPI& bind_srv(RHIShaderResourceView* view, byte slot) override;
		VulkanAPI& bind_uav(RHIUnorderedAccessView* view, byte slot) override;
		VulkanAPI& bind_sampler(RHISampler* view, byte slot) override;

		VulkanAPI& barrier(RHITexture* texture, RHIAccess dst_access) override;
		VulkanAPI& barrier(RHIBuffer* buffer, RHIAccess dst_access) override;

		VulkanAPI& begin_timestamp(RHITimestamp* timestamp) override;
		VulkanAPI& end_timestamp(RHITimestamp* timestamp) override;

		VulkanAPI& begin_statistics(RHIPipelineStatistics* stats) override;
		VulkanAPI& end_statistics(RHIPipelineStatistics* stats) override;

		VulkanAPI& present(RHISwapchain* swapchain) override;

		~VulkanAPI();
	};
}// namespace Engine
