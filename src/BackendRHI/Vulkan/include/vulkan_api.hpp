#pragma once
#include <Core/etl/array.hpp>
#include <Core/etl/atomic.hpp>
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
			PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT                       = nullptr;
			PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT                           = nullptr;
			PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR             = nullptr;
			PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR               = nullptr;
			PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT                                     = nullptr;
			PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR                     = nullptr;
			PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR                         = nullptr;
			PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
			PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR               = nullptr;
			PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR             = nullptr;
			PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR         = nullptr;
			PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR                   = nullptr;
			PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR       = nullptr;
			PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR                                             = nullptr;

			inline uint32_t getVkHeaderVersion() const { return VK_HEADER_VERSION; }
		} pfn;

		// API DATA
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

		Atomic<uint64_t> m_frame;

		Deque<Garbage> m_garbage;
		Vector<VulkanExtention> m_device_extensions;
		MultiMap<uint64_t, class VulkanPipelineLayout*> m_pipeline_layouts;
		VulkanDescriptorSetAllocator* m_descriptor_set_allocator;
		VulkanQueryPoolManager* m_query_pool_manager;
		VulkanDescriptorHeap* m_descriptor_heap;
		VulkanUpdater* m_updater = nullptr;

		vk::Semaphore m_timeline;

		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_ray_trace_properties;

	private:
		static consteval auto make_extensions_array()
		{
			return std::to_array<VulkanExtention>({
			        {"", false},// Dummy extension
			        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true},
			        {VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, true},
			        {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false},
			        {VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, false},
			        {VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME, false},
			        {VK_KHR_SPIRV_1_4_EXTENSION_NAME, false},
			        {VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, false},
			        {VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, false},
			        {VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false},
			        {VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, false},
			        {VK_KHR_RAY_QUERY_EXTENSION_NAME, false},
			        {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, true},
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
		inline VulkanQueryPoolManager* query_pool_manager() const { return m_query_pool_manager; }
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
		//////////////////////////////////////////////////////////////

		// API METHODS

		vk::SurfaceKHR create_surface(Window* interface);
		void initialize_pfn();

		vk::Extent2D surface_size(const vk::SurfaceKHR& surface) const;
		bool has_stencil_component(vk::Format format);

		bool is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal);
		VulkanAPI& add_garbage(RHIObject* object);
		//////////////////////////////////////////////////////////////

		VulkanAPI();

		VulkanAPI& signal(RHIFence* fence) override;
		VulkanAPI& submit(RHICommandHandle* cmd) override;
		VulkanAPI& wait_idle();

		vk::PresentModeKHR present_mode_of(bool vsync, vk::SurfaceKHR surface);

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
		RHIPipeline* create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline) override;
		RHIBuffer* create_buffer(size_t size, const byte* data, RHIBufferCreateFlags flags) override;
		RHISwapchain* create_swapchain(Window* window, bool vsync) override;
		RHIContext* create_context() override;

		RHIAccelerationStructure* create_acceleration_structure(const RHIRayTracingAccelerationInputs* inputs) override;
		const byte* translate_ray_tracing_instances(const RHIRayTracingGeometryInstance* instances, size_t& size) override;

		VulkanAPI& present(RHISwapchain* swapchain) override;

		~VulkanAPI();
	};
}// namespace Engine
