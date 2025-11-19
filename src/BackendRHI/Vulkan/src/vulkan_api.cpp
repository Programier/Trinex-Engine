#include <VkBootstrap.h>

#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/struct.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_config.hpp>
#include <vulkan_context.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_query.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_thread_local.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
	VulkanAPI* VulkanAPI::m_vulkan = nullptr;

	namespace TRINEX_RHI
	{
		using VULKAN = VulkanAPI;
	}

	trinex_implement_struct_default_init(Engine::TRINEX_RHI::VULKAN, 0);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	                                                     VkDebugUtilsMessageTypeFlagsEXT message_type,
	                                                     const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	                                                     void* pUserData)
	{
#define has_bit(bit) ((message_severity & bit) == bit)

		if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) || has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
		{
			info_log("Vulkan API", "%s", callback_data->pMessage);
		}
		else if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
		{
			warn_log("Vulkan API", "%s", callback_data->pMessage);
		}
		else if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
		{
			error_log("Vulkan API", "%s", callback_data->pMessage);
		}

		return VK_FALSE;
#undef has_bit
	}

	static vkb::PhysicalDevice initialize_physical_device()
	{
		vkb::PhysicalDeviceSelector phys_device_selector(API->m_instance);

		for (const VulkanExtention& extension : API->extensions())
		{
			if (extension.is_valid() && extension.enabled)
			{
				phys_device_selector.add_required_extension(extension.name.data());
			}
		}

		phys_device_selector.defer_surface_initialization();
		phys_device_selector.allow_any_gpu_device_type(VulkanConfig::allow_any_gpu_type);
		phys_device_selector.require_present();
		phys_device_selector.prefer_gpu_device_type(VulkanConfig::device_type);

		auto selected_device = phys_device_selector.select();
		if (!selected_device.has_value())
		{
			auto msg = selected_device.error().message();
			error_log("Vulkan", "%s", msg.c_str());
			throw std::runtime_error(msg);
		}

		auto device = selected_device.value();

		for (const VulkanExtention& extension : API->extensions())
		{
			if (extension.is_valid() && !extension.enabled)
			{
				extension.enabled = device.enable_extension_if_present(extension.name.data());
			}
		}

		return device;
	}

	static void clean_pnext(vk::BaseOutStructure* base)
	{
		while (base)
		{
			vk::BaseOutStructure* next = base->pNext;
			base->pNext                = nullptr;
			base                       = next;
		}
	}

	static vkb::Device build_device(vkb::PhysicalDevice& physical_device)
	{
		vkb::DeviceBuilder builder(physical_device);

		vk::PhysicalDeviceFeatures2 features;
		vk::PhysicalDeviceCustomBorderColorFeaturesEXT custom_border;
		vk::PhysicalDeviceVulkan11Features vk11_features;
		vk::PhysicalDeviceMeshShaderFeaturesEXT mesh_shaders;
		vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing;
		vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore;
		vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR device_address;
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR acceleration;
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing;
		vk::PhysicalDeviceFragmentShadingRateFeaturesKHR shading_rate;

		features.pNext            = &custom_border;
		custom_border.pNext       = &vk11_features;
		vk11_features.pNext       = &mesh_shaders;
		mesh_shaders.pNext        = &descriptor_indexing;
		descriptor_indexing.pNext = &timeline_semaphore;
		timeline_semaphore.pNext  = &device_address;
		device_address.pNext      = &acceleration;
		acceleration.pNext        = &ray_tracing;
		ray_tracing.pNext         = &shading_rate;

		vk::PhysicalDevice(physical_device.physical_device).getFeatures2(&features);
		clean_pnext(reinterpret_cast<vk::BaseOutStructure*>(&features));

		mesh_shaders.primitiveFragmentShadingRateMeshShader               = vk::False;
		acceleration.accelerationStructureCaptureReplay                   = vk::False;
		ray_tracing.rayTracingPipelineShaderGroupHandleCaptureReplay      = vk::False;
		ray_tracing.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = vk::False;

		builder.add_pNext(&vk11_features);
		builder.add_pNext(&descriptor_indexing);
		builder.add_pNext(&timeline_semaphore);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME)))
			builder.add_pNext(&custom_border);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_EXT_MESH_SHADER_EXTENSION_NAME)))
			builder.add_pNext(&mesh_shaders);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)))
			builder.add_pNext(&device_address);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)))
			builder.add_pNext(&acceleration);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)))
			builder.add_pNext(&ray_tracing);

		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)))
			builder.add_pNext(&shading_rate);

		auto device_ret = builder.build();

		if (!device_ret)
		{
			throw std::runtime_error(device_ret.error().message());
		}

		return device_ret.value();
	}

	static vk::PhysicalDeviceFeatures filter_features(const vk::PhysicalDeviceFeatures& features)
	{
		vk::PhysicalDeviceFeatures new_features;

		new_features.samplerAnisotropy                    = features.samplerAnisotropy;
		new_features.fillModeNonSolid                     = features.fillModeNonSolid;
		new_features.wideLines                            = features.wideLines;
		new_features.tessellationShader                   = features.tessellationShader;
		new_features.geometryShader                       = features.geometryShader;
		new_features.shaderStorageImageReadWithoutFormat  = features.shaderStorageImageReadWithoutFormat;
		new_features.shaderStorageImageWriteWithoutFormat = features.shaderStorageImageWriteWithoutFormat;
		new_features.shaderInt16                          = features.shaderInt16;
		new_features.shaderInt64                          = features.shaderInt64;
		new_features.pipelineStatisticsQuery              = features.pipelineStatisticsQuery;

		return new_features;
	}

	struct VulkanAPI::VulkanUpdater : TickableObject {
		VulkanUpdater& update(float dt) override
		{
			API->update(dt);
			return *this;
		}
	};

	VulkanAPI* VulkanAPI::static_constructor()
	{
		if (VulkanAPI::m_vulkan == nullptr)
		{
			auto& info           = (trx_new VulkanAPI())->info;
			info.name            = "Vulkan";
			info.struct_instance = static_reflection();
		}
		return VulkanAPI::m_vulkan;
	}

	void VulkanAPI::static_destructor(VulkanAPI* vulkan)
	{
		if (vulkan == m_vulkan)
		{
			trx_delete vulkan;
			m_vulkan = nullptr;
		}
	}

	VulkanAPI::VulkanAPI()
	{
		m_vulkan   = this;
		auto array = make_extensions_array();
		m_device_extensions.assign(array.begin(), array.end());

		vkb::InstanceBuilder instance_builder;
		instance_builder.require_api_version(VK_API_VERSION_1_1);

		if (VulkanConfig::enable_validation)
		{
			instance_builder.set_debug_callback(debug_callback)
			        .request_validation_layers(true)
			        .enable_validation_layers(true)
			        .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
		}
		else
		{
			instance_builder.enable_validation_layers(false).request_validation_layers(false);
		}

		auto instance_ret = instance_builder.build();

		if (!instance_ret)
		{
			auto message = instance_ret.error().message();
			error_log("Vulkan", "%s", message.c_str());
			throw EngineException(message);
		}

		m_instance = instance_ret.value();

		// Initialize physical device
		auto selected_device = initialize_physical_device();
		m_physical_device    = vk::PhysicalDevice(selected_device.physical_device);

		m_properties  = m_physical_device.getProperties();
		m_features    = filter_features(m_physical_device.getFeatures());
		info.renderer = m_properties.deviceName.data();
		selected_device.enable_features_if_present(m_features);

		if (is_raytracing_supported())
		{
			vk::PhysicalDeviceProperties2 props({}, &m_ray_trace_properties);
			m_physical_device.getProperties2(&props);
		}

		info_log("Vulkan", "Selected GPU '%s'", info.renderer.c_str());

		// Initialize device
		auto bootstrap_device = build_device(selected_device);
		m_device              = vk::Device(bootstrap_device.device);

		auto graphics_queue_index = bootstrap_device.get_queue_index(vkb::QueueType::graphics);
		auto graphics_queue       = bootstrap_device.get_queue(vkb::QueueType::graphics);

		if (!graphics_queue_index.has_value() || !graphics_queue.has_value())
		{
			throw EngineException("Failed to create graphics queue");
		}

		m_graphics_queue = trx_new VulkanQueue(graphics_queue.value(), graphics_queue_index.value());

		initialize_pfn();

		m_stagging_manager        = trx_new VulkanStaggingBufferManager();
		m_pipeline_layout_manager = trx_new VulkanPipelineLayoutManager();
		m_query_pool_manager      = trx_new VulkanQueryPoolManager();
		m_descriptor_heap         = trx_new VulkanDescriptorHeap();
		m_updater                 = trx_new VulkanUpdater();

		// Initialize memory allocator
		{
			VmaVulkanFunctions vulkan_functions    = {};
			vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
			vulkan_functions.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

			VmaAllocatorCreateInfo allocator_info = {};
			allocator_info.vulkanApiVersion       = VK_API_VERSION_1_0;
			allocator_info.physicalDevice         = m_physical_device;
			allocator_info.instance               = m_instance;
			allocator_info.device                 = m_device;
			allocator_info.pVulkanFunctions       = &vulkan_functions;

			allocator_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

			vmaCreateAllocator(&allocator_info, &m_allocator);
		}

		{
			vk::SemaphoreTypeCreateInfo type_info(vk::SemaphoreType::eTimeline, 0);
			vk::SemaphoreCreateInfo create_info({}, &type_info);
			m_timeline = m_device.createSemaphore(create_info);
		}
	}

	VulkanAPI::~VulkanAPI()
	{
		idle();

		VulkanRenderPass::destroy_all();

		destroy_garbage();

		for (VulkanThreadLocal* local : m_thread_locals)
		{
			trx_delete local;
		}

		trx_delete m_updater;
		trx_delete m_stagging_manager;
		trx_delete m_pipeline_layout_manager;
		trx_delete m_graphics_queue;
		trx_delete m_query_pool_manager;
		trx_delete m_descriptor_heap;

		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;

		m_device.destroySemaphore(m_timeline);

		m_device.destroy();
		vkb::destroy_instance(m_instance);
	}

	VulkanAPI& VulkanAPI::update(float dt)
	{
		const uint64_t current_frame = ++m_frame;
		const uint64_t gpu_frame     = m_device.getSemaphoreCounterValueKHR(m_timeline, pfn);

		while (!m_garbage.empty() && m_garbage.front().frame <= gpu_frame)
		{
			m_garbage.front().object->destroy();
			m_garbage.pop_front();
		}

		vk::TimelineSemaphoreSubmitInfo timeline_info(0, nullptr, 1, &current_frame);

		vk::SubmitInfo submit_info;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &m_timeline;
		submit_info.pNext                = &timeline_info;

		m_graphics_queue->submit(submit_info);
		m_stagging_manager->update();
		return *this;
	}

	VulkanAPI& VulkanAPI::destroy_garbage()
	{
		while (!m_garbage.empty())
		{
			m_garbage.front().object->destroy();
			m_garbage.pop_front();
		}

		return *this;
	}

	static vk::PresentModeKHR find_present_mode(const std::vector<vk::PresentModeKHR>& modes,
	                                            const std::initializer_list<vk::PresentModeKHR>& requested)
	{
		for (auto& mode : requested)
		{
			if (std::find(modes.begin(), modes.end(), mode) != modes.end())
				return mode;
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::PresentModeKHR VulkanAPI::present_mode_of(bool vsync, vk::SurfaceKHR surface)
	{
		auto present_modes = m_physical_device.getSurfacePresentModesKHR(surface);

		if (present_modes.empty())
			throw EngineException("Failed to find present mode!");

		if (vsync)
		{
			return find_present_mode(present_modes, {vk::PresentModeKHR::eFifo, vk::PresentModeKHR::eFifoRelaxed});
		}
		else
		{
			return find_present_mode(present_modes, {vk::PresentModeKHR::eImmediate, vk::PresentModeKHR::eMailbox});
		}
	}

	void VulkanAPI::initialize_pfn()
	{
		auto load = [this]<typename T>(T& func, const char* name) {
			func = reinterpret_cast<T>(vkGetDeviceProcAddr(m_device, name));
		};

		load(pfn.vkCmdBeginDebugUtilsLabelEXT, "vkCmdBeginDebugUtilsLabelEXT");
		load(pfn.vkCmdEndDebugUtilsLabelEXT, "vkCmdEndDebugUtilsLabelEXT");
		load(pfn.vkGetBufferMemoryRequirements2KHR, "vkGetBufferMemoryRequirements2KHR");
		load(pfn.vkCmdDrawMeshTasksEXT, "vkCmdDrawMeshTasksEXT");
		load(pfn.vkGetSemaphoreCounterValueKHR, "vkGetSemaphoreCounterValueKHR");
		load(pfn.vkGetBufferDeviceAddressKHR, "vkGetBufferDeviceAddressKHR");
		load(pfn.vkGetAccelerationStructureBuildSizesKHR, "vkGetAccelerationStructureBuildSizesKHR");
		load(pfn.vkCreateAccelerationStructureKHR, "vkCreateAccelerationStructureKHR");
		load(pfn.vkDestroyAccelerationStructureKHR, "vkDestroyAccelerationStructureKHR");
		load(pfn.vkCmdBuildAccelerationStructuresKHR, "vkCmdBuildAccelerationStructuresKHR");
		load(pfn.vkCreateRayTracingPipelinesKHR, "vkCreateRayTracingPipelinesKHR");
		load(pfn.vkGetRayTracingShaderGroupHandlesKHR, "vkGetRayTracingShaderGroupHandlesKHR");
		load(pfn.vkCmdTraceRaysKHR, "vkCmdTraceRaysKHR");
		load(pfn.vkCmdSetFragmentShadingRateKHR, "vkCmdSetFragmentShadingRateKHR");
	}

	vk::SurfaceKHR VulkanAPI::create_surface(Window* window)
	{
		extern vk::SurfaceKHR create_vulkan_surface(void* native_window, vk::Instance instance);
		return create_vulkan_surface(window->native_window(), m_instance.instance);
	}

	vk::Extent2D VulkanAPI::surface_size(const vk::SurfaceKHR& surface) const
	{
		return m_physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
	}

	bool VulkanAPI::has_stencil_component(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	bool VulkanAPI::is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal)
	{
		vk::FormatProperties properties            = m_physical_device.getFormatProperties(format);
		const vk::FormatFeatureFlags feature_flags = optimal ? properties.optimalTilingFeatures : properties.linearTilingFeatures;
		return (feature_flags & flags) == flags;
	}

	VulkanAPI& VulkanAPI::submit(RHICommandHandle* cmd)
	{
		vk::CommandBuffer& buffer = *static_cast<VulkanCommandHandle*>(cmd);
		vk::SubmitInfo info({}, {}, buffer, {});
		m_graphics_queue->submit(info);
		return *this;
	}

	VulkanAPI& VulkanAPI::idle()
	{
		m_graphics_queue->idle();
		return *this;
	}

	VulkanAPI& VulkanAPI::add_garbage(RHIObject* object)
	{
		Garbage garbage;
		garbage.object = object;
		garbage.frame  = m_frame + 5;

		m_garbage.push_back(garbage);
		return *this;
	}

	void trinex_vulkan_deferred_destroy(RHIObject* object)
	{
		API->add_garbage(object);
	}
}// namespace Engine
