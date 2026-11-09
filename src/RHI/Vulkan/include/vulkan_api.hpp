#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/constexpr_string.hpp>
#include <Core/etl/deque.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/static_array.hpp>
#include <RHI/rhi.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanBuffer;
	class VulkanTexture;
	class VulkanQueue;
	class VulkanCommandBufferManager;
	class VulkanPipelineLayoutManager;
	class VulkanUniformBufferManager;
	class VulkanDescriptorSetAllocator;
	class VulkanStateManager;
	class VulkanQueryPoolManager;
	class VulkanDescriptorHeap;
	class Window;

	struct VulkanExtention
	{
		StringView name;
		mutable bool enabled = false;

		inline bool operator==(const VulkanExtention& ext) const { return name == ext.name; }
		inline bool operator!=(const VulkanExtention& ext) const { return name != ext.name; }
		inline bool is_valid() const { return !name.empty(); }
	};

	class VulkanAPI final : public RHI
	{
	public:
		vk::Instance m_instance;
		vk::PhysicalDevice m_physical_device;
		VmaAllocator m_allocator = VK_NULL_HANDLE;

		vk::PhysicalDeviceProperties m_properties;
		vk::PhysicalDeviceFeatures m_features;

	private:
		static VulkanAPI* s_vulkan;

		vk::Device m_device;

		VulkanQueue* m_graphics_queue = nullptr;
		VulkanQueue* m_compute_queue  = nullptr;
		VulkanQueue* m_transfer_queue = nullptr;
		Array<RHIQueue*> m_queues;

		Array<VulkanExtention> m_device_extensions;

		VulkanQueryPoolManager* m_query_pool_manager;
		VulkanPipelineLayoutManager* m_pipeline_layout_manager;
		VulkanDescriptorHeap* m_descriptor_heap;
		vk::Semaphore m_timeline;
		vk::DebugUtilsMessengerEXT m_debug_messenger;

		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_ray_trace_properties;

	private:
		static consteval auto make_extensions_array()
		{
			return etl::to_array<VulkanExtention>({
			        {"", false}, // Dummy extension
			        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
			        {VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, true},
			        {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false},
			        {VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, false},
			        {VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, false},
			        {VK_KHR_SPIRV_1_4_EXTENSION_NAME, false},
			        {VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, false},
			        {VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, false},
			        // {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false},
			        // {VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, false},
			        // {VK_KHR_RAY_QUERY_EXTENSION_NAME, false},
			        {VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, true},
			        {VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, true},
			        {VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, true},
			        {VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, false},
			        {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, true},
			        {VK_EXT_MESH_SHADER_EXTENSION_NAME, false},
			});
		}

	public:
		static consteval usize find_extension_index(const char* str)
		{
			auto extensions = make_extensions_array();

			for (usize i = 1, count = extensions.size(); i < count; ++i)
			{
				if (extensions[i].name == str)
					return i;
			}

			return 0;
		}

		friend class VulkanThreadLocal;

	public:
		inline const Array<VulkanExtention>& extensions() const { return m_device_extensions; }
		inline bool is_extension_enabled(usize index) const { return m_device_extensions[index].enabled; }

		template<ConstexprString ext>
		inline bool is_extension_enabled() const
		{
			return is_extension_enabled(find_extension_index(ext.c_str()));
		}

		inline VulkanDescriptorHeap* descriptor_heap() const { return m_descriptor_heap; }
		inline VulkanQueryPoolManager* query_pool_manager() const { return m_query_pool_manager; }
		inline VulkanPipelineLayoutManager* pipeline_layout_manager() const { return m_pipeline_layout_manager; }
		inline const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR& ray_trace_properties() const
		{
			return m_ray_trace_properties;
		}

		inline bool is_raytracing_supported() const
		{
			if (!is_extension_enabled(find_extension_index(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)))
				return false;

			if (!is_extension_enabled(find_extension_index(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)))
				return false;

			if (!is_extension_enabled(find_extension_index(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)))
				return false;

			return true;
		}

	public:
		vk::SurfaceKHR create_surface(Window* interface);
		vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;
		bool has_stencil_component(vk::Format format);

		bool is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal);

	public:
		static inline VulkanAPI* instance() { return s_vulkan; }
		static inline vk::Device* device() { return &instance()->m_device; }

		static inline VulkanQueue* graphics() { return instance()->m_graphics_queue; }
		static inline VulkanQueue* compute() { return instance()->m_compute_queue; }
		static inline VulkanQueue* transfer() { return instance()->m_transfer_queue; }

	public:
		VulkanAPI();
		~VulkanAPI();

		const char* name() const override { return "Vulkan"; }
		const char* renderer() const override { return m_properties.deviceName.data(); }

		void wait() override;
		RHIQueue* queue(RHIQueueType type, RHISwapchain* swapchain = nullptr) override;
		RHIArrayView<RHIQueue*> queues() const override;

		vk::PresentModeKHR present_mode_of(RHIPresentMode mode, vk::SurfaceKHR surface);

		RHIMemory* create_memory(usize size, RHIMemoryType type) override;
		RHITimestamp* create_timestamp() override;
		RHIPipelineStatistics* create_pipeline_statistics() override;
		RHIFence* create_fence(u64 value = 0) override;
		RHISampler* create_sampler(const RHISamplerDesc& desc) override;
		RHITexture* create_texture(const RHITextureDesc& desc) override;
		RHIShader* create_shader(const void* data, usize size) override;
		RHIPipeline* create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc) override;
		RHIPipeline* create_mesh_pipeline(const RHIMeshPipelineDesc& desc) override;
		RHIPipeline* create_compute_pipeline(const RHIComputePipelineDesc& desc) override;
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc) override;
		RHIBuffer* create_buffer(usize size, RHIBufferFlags flags) override;
		RHISwapchain* create_swapchain(Window* window, RHIPresentMode mode, u32 images = 0) override;
		RHIContext* create_context(RHIQueue* queue) override;
		RHIBindingLayout* create_binding_layout(RHIBinding* bindings = nullptr, usize count = 0) override;
		RHIBindingSet* create_binding_set(RHIBindingLayout* layout) override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const u8* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, usize& size) override;
	};
} // namespace Trinex
