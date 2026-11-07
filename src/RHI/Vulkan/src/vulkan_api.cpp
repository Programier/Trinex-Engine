#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
#include <Core/tickable.hpp>
#include <Core/window.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <SDL3/SDL_vulkan.h>
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
#include <vulkan_shader.hpp>
#include <vulkan_sync.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_thread_local.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Trinex
{
	static RHIRegistry<VulkanAPI> s_vulkan_api_registry(RHIBackend::Vulkan);
	VulkanAPI* VulkanAPI::s_vulkan = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	                                                     VkDebugUtilsMessageTypeFlagsEXT message_type,
	                                                     const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
	                                                     void* pUserData)
	{
#define has_bit(bit) ((message_severity & bit) == bit)

		if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
		{
			trinex_debug(Log::RHI, "%s", callback_data->pMessage);
		}
		else if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT))
		{
			trinex_warning(Log::RHI, "%s", callback_data->pMessage);
		}
		else if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT))
		{
			trinex_error(Log::RHI, "%s", callback_data->pMessage);
		}

		return VK_FALSE;
#undef has_bit
	}

	struct PhysicalDeviceSelection
	{
		vk::PhysicalDevice device;
		u32 graphics_queue_family;
		u32 compute_queue_family;
		u32 transfer_queue_family;
	};

	static constexpr u32 invalid_queue_family = ~0u;

	template<typename Families>
	static u32 find_queue_family(const Families& families, vk::QueueFlags required, vk::QueueFlags excluded = {})
	{
		for (u32 index = 0; index < families.size(); ++index)
		{
			const vk::QueueFamilyProperties& family = families[index];
			if (family.queueCount && (family.queueFlags & required) == required && !(family.queueFlags & excluded))
				return index;
		}

		return invalid_queue_family;
	}

	static RHIQueueFlags rhi_queue_flags(vk::QueueFlags flags)
	{
		RHIQueueFlags result = {};

		if (flags & vk::QueueFlagBits::eGraphics)
			result |= RHIQueueFlags::Graphics;
		if (flags & vk::QueueFlagBits::eCompute)
			result |= RHIQueueFlags::Compute;
		if (flags & vk::QueueFlagBits::eTransfer)
			result |= RHIQueueFlags::Transfer;

		return result;
	}

	static const char* rhi_queue_name(RHIQueueType type)
	{
		switch (type)
		{
			case RHIQueueType::Graphics: return "Graphics";
			case RHIQueueType::Compute: return "Compute";
			case RHIQueueType::Transfer: return "Transfer";
			case RHIQueueType::Present: return "Present";
			default: return "Undefined";
		}
	}

	template<typename Extensions>
	static bool contains_extension(const Extensions& extensions, StringView name)
	{
		for (const auto& extension : extensions)
		{
			if (name == extension.extensionName)
				return true;
		}
		return false;
	}

	static bool contains_extension(const Array<const char*>& extensions, StringView name)
	{
		for (const char* extension : extensions)
		{
			if (name == extension)
				return true;
		}
		return false;
	}

	static void add_instance_extension(Array<const char*>& enabled_extensions, const auto& available_extensions, const char* name)
	{
		if (!contains_extension(available_extensions, name) || contains_extension(enabled_extensions, name))
			return;

		enabled_extensions.push_back(name);
	}

	static void add_surface_extensions(Array<const char*>& enabled_extensions)
	{
		auto available_extensions = vk::verify(vk::enumerateInstanceExtensionProperties());

		add_instance_extension(enabled_extensions, available_extensions, VK_KHR_SURFACE_EXTENSION_NAME);

#if PLATFORM_WINDOWS
		add_instance_extension(enabled_extensions, available_extensions, "VK_KHR_win32_surface");
#elif PLATFORM_ANDROID
		add_instance_extension(enabled_extensions, available_extensions, "VK_KHR_android_surface");
#elif PLATFORM_LINUX
		add_instance_extension(enabled_extensions, available_extensions, "VK_KHR_xlib_surface");
		add_instance_extension(enabled_extensions, available_extensions, "VK_KHR_wayland_surface");
#endif
	}

	static PhysicalDeviceSelection initialize_physical_device()
	{
		VulkanAPI* api = VulkanAPI::instance();
		auto devices   = vk::verify(api->m_instance.enumeratePhysicalDevices());

		PhysicalDeviceSelection selected = {};
		bool has_selected                = false;

		for (vk::PhysicalDevice device : devices)
		{
			const vk::PhysicalDeviceProperties properties = device.getProperties();
			if (!VulkanConfig::allow_any_gpu_type
			    && properties.deviceType != static_cast<vk::PhysicalDeviceType>(VulkanConfig::device_type))
				continue;

			auto extensions = vk::verify(device.enumerateDeviceExtensionProperties());
			bool supported  = true;

			for (const VulkanExtention& extension : api->extensions())
			{
				if (extension.is_valid() && extension.enabled && !contains_extension(extensions, extension.name))
				{
					supported = false;
					break;
				}
			}

			if (!supported)
				continue;

			auto queue_families = device.getQueueFamilyProperties();
			const u32 graphics  = find_queue_family(queue_families, vk::QueueFlagBits::eGraphics);
			if (graphics != invalid_queue_family)
			{
				u32 compute = find_queue_family(queue_families, vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
				if (compute == invalid_queue_family)
					compute = find_queue_family(queue_families, vk::QueueFlagBits::eCompute);

				u32 transfer = find_queue_family(queue_families, vk::QueueFlagBits::eTransfer,
				                                 vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);
				if (transfer == invalid_queue_family)
					transfer = find_queue_family(queue_families, vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics);
				if (transfer == invalid_queue_family)
					transfer = find_queue_family(queue_families, vk::QueueFlagBits::eTransfer);

				selected = {
				        .device                = device,
				        .graphics_queue_family = graphics,
				        .compute_queue_family  = compute == invalid_queue_family ? graphics : compute,
				        .transfer_queue_family = transfer == invalid_queue_family ? graphics : transfer,
				};
				has_selected = true;
			}

			if (has_selected && properties.deviceType == static_cast<vk::PhysicalDeviceType>(VulkanConfig::device_type))
				break;
		}

		trinex_verify_msg(has_selected, "Failed to find a suitable Vulkan physical device");

		auto extensions = vk::verify(selected.device.enumerateDeviceExtensionProperties());
		for (const VulkanExtention& extension : api->extensions())
		{
			if (extension.is_valid() && !extension.enabled)
				extension.enabled = contains_extension(extensions, extension.name);
		}

		return selected;
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

	static vk::Device build_device(const PhysicalDeviceSelection& selection)
	{
		VulkanAPI* api = VulkanAPI::instance();

		vk::PhysicalDeviceFeatures2 features;
		vk::PhysicalDeviceVulkan11Features vk11_features;
		vk::PhysicalDeviceMeshShaderFeaturesEXT mesh_shaders;
		vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing;
		vk::PhysicalDeviceTimelineSemaphoreFeatures timeline_semaphore;
		vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR device_address;
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR acceleration;
		vk::PhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing;
		vk::PhysicalDeviceFragmentShadingRateFeaturesKHR shading_rate;
		vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering;

		features.pNext            = &vk11_features;
		vk11_features.pNext       = &mesh_shaders;
		mesh_shaders.pNext        = &descriptor_indexing;
		descriptor_indexing.pNext = &timeline_semaphore;
		timeline_semaphore.pNext  = &device_address;
		device_address.pNext      = &acceleration;
		acceleration.pNext        = &ray_tracing;
		ray_tracing.pNext         = &shading_rate;
		shading_rate.pNext        = &dynamic_rendering;

		api->m_physical_device.getFeatures2(&features);
		clean_pnext(reinterpret_cast<vk::BaseOutStructure*>(&features));
		features.features = api->m_features;

		mesh_shaders.primitiveFragmentShadingRateMeshShader               = vk::False;
		acceleration.accelerationStructureCaptureReplay                   = vk::False;
		ray_tracing.rayTracingPipelineShaderGroupHandleCaptureReplay      = vk::False;
		ray_tracing.rayTracingPipelineShaderGroupHandleCaptureReplayMixed = vk::False;

		features.pNext            = &vk11_features;
		vk11_features.pNext       = &descriptor_indexing;
		descriptor_indexing.pNext = &timeline_semaphore;
		timeline_semaphore.pNext  = &dynamic_rendering;
		dynamic_rendering.pNext   = nullptr;

		if (api->is_extension_enabled<VK_EXT_MESH_SHADER_EXTENSION_NAME>())
		{
			dynamic_rendering.pNext = &mesh_shaders;
			mesh_shaders.pNext      = nullptr;
		}

		vk::BaseOutStructure* chain = reinterpret_cast<vk::BaseOutStructure*>(&dynamic_rendering);
		while (chain->pNext)
			chain = chain->pNext;

		if (api->is_extension_enabled<VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME>())
		{
			chain->pNext = reinterpret_cast<vk::BaseOutStructure*>(&device_address);
			chain        = chain->pNext;
		}

		if (api->is_extension_enabled<VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME>())
		{
			chain->pNext = reinterpret_cast<vk::BaseOutStructure*>(&acceleration);
			chain        = chain->pNext;
		}

		if (api->is_extension_enabled<VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME>())
		{
			chain->pNext = reinterpret_cast<vk::BaseOutStructure*>(&ray_tracing);
			chain        = chain->pNext;
		}

		if (api->is_extension_enabled<VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME>())
			chain->pNext = reinterpret_cast<vk::BaseOutStructure*>(&shading_rate);

		Array<const char*> extensions;
		for (const VulkanExtention& extension : api->extensions())
		{
			if (extension.is_valid() && extension.enabled)
				extensions.push_back(extension.name.data());
		}

		const float priority = 1.f;
		Array<u32> queue_families;
		for (u32 family : {selection.graphics_queue_family, selection.compute_queue_family, selection.transfer_queue_family})
		{
			bool exists = false;
			for (u32 existing : queue_families)
			{
				if (existing == family)
				{
					exists = true;
					break;
				}
			}

			if (!exists)
				queue_families.push_back(family);
		}

		Array<vk::DeviceQueueCreateInfo> queue_infos;
		queue_infos.reserve(queue_families.size());
		for (u32 family : queue_families)
			queue_infos.emplace_back(vk::DeviceQueueCreateFlags{}, family, 1, &priority);

		vk::DeviceCreateInfo info({}, queue_infos.size(), queue_infos.data(), 0, nullptr, extensions.size(), extensions.data(),
		                          nullptr, &features);

		return vk::verify(api->m_physical_device.createDevice(info));
	}

	static vk::PhysicalDeviceFeatures filter_features(const vk::PhysicalDeviceFeatures& features)
	{
		vk::PhysicalDeviceFeatures new_features;

		new_features.samplerAnisotropy                    = features.samplerAnisotropy;
		new_features.fillModeNonSolid                     = features.fillModeNonSolid;
		new_features.tessellationShader                   = features.tessellationShader;
		new_features.geometryShader                       = features.geometryShader;
		new_features.shaderStorageImageReadWithoutFormat  = features.shaderStorageImageReadWithoutFormat;
		new_features.shaderStorageImageWriteWithoutFormat = features.shaderStorageImageWriteWithoutFormat;
		new_features.shaderInt16                          = features.shaderInt16;
		new_features.shaderInt64                          = features.shaderInt64;
		new_features.pipelineStatisticsQuery              = features.pipelineStatisticsQuery;
		new_features.vertexPipelineStoresAndAtomics       = features.vertexPipelineStoresAndAtomics;
		new_features.fragmentStoresAndAtomics             = features.fragmentStoresAndAtomics;

		return new_features;
	}

	VulkanAPI::VulkanAPI()
	{
		s_vulkan   = this;
		auto array = make_extensions_array();
		m_device_extensions.assign(array.begin(), array.end());

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		Array<const char*> instance_extensions;
		u32 extension_count           = 0;
		const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extension_count);
		trinex_verify_msg(extensions, "Failed to get required Vulkan instance extensions from SDL");
		instance_extensions.insert(instance_extensions.end(), extensions, extensions + extension_count);
		add_surface_extensions(instance_extensions);

		vk::DebugUtilsMessengerCreateInfoEXT debug_info;
		vk::ValidationFeaturesEXT validation_info;
		Array<const char*> validation_layers;

		if (VulkanConfig::enable_validation)
		{
			validation_layers.push_back("VK_LAYER_KHRONOS_validation");
			instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instance_extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

			debug_info
			        .setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
			                            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
			                            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			                            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			                        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			                        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
			        .setPfnUserCallback(reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debug_callback));

			const vk::ValidationFeatureEnableEXT validation_features[] = {
			        vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
			};
			validation_info.setEnabledValidationFeatures(validation_features);
			debug_info.pNext = &validation_info;
		}

		vk::ApplicationInfo application_info("Trinex Engine", VK_MAKE_API_VERSION(0, 0, 0, 1), "Trinex",
		                                     VK_MAKE_API_VERSION(0, 0, 0, 1), VK_API_VERSION_1_1);
		vk::InstanceCreateInfo instance_info({}, &application_info, validation_layers.size(), validation_layers.data(),
		                                     instance_extensions.size(), instance_extensions.data(),
		                                     VulkanConfig::enable_validation ? &debug_info : nullptr);

		m_instance = vk::verify(vk::createInstance(instance_info));
		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

		if (VulkanConfig::enable_validation)
		{
			debug_info.pNext  = nullptr;
			m_debug_messenger = vk::verify(m_instance.createDebugUtilsMessengerEXT(debug_info));
		}

		// Initialize physical device
		auto selected_device = initialize_physical_device();
		m_physical_device    = selected_device.device;

		m_properties = m_physical_device.getProperties();
		m_features   = filter_features(m_physical_device.getFeatures());

		if (is_raytracing_supported())
		{
			vk::PhysicalDeviceProperties2 props({}, &m_ray_trace_properties);
			m_physical_device.getProperties2(&props);
		}

		trinex_info(Log::RHI, "Selected GPU '%s'", renderer());

		// Initialize device
		m_device = build_device(selected_device);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);

		const auto queue_families = m_physical_device.getQueueFamilyProperties();
		auto create_queue         = [this, &queue_families](u32 family) {
			for (RHIQueue* base_queue : m_queues)
			{
				auto* queue = static_cast<VulkanQueue*>(base_queue);
				if (queue->family_index() == family && queue->queue_index() == 0)
					return queue;
			}

			VulkanQueue* queue = trx_new VulkanQueue(m_device.getQueue(family, 0),
			                                         rhi_queue_flags(queue_families[family].queueFlags), family, 0);
			m_queues.push_back(queue);
			return queue;
		};

		m_graphics_queue = create_queue(selected_device.graphics_queue_family);
		m_compute_queue  = create_queue(selected_device.compute_queue_family);
		m_transfer_queue = create_queue(selected_device.transfer_queue_family);

		auto log_queue = [](RHIQueueType type, const VulkanQueue* queue) {
			trinex_info(Log::RHI, "Created %s queue (family %u, index %u)", rhi_queue_name(type), queue->family_index(),
			            queue->queue_index());
		};

		log_queue(RHIQueueType::Graphics, m_graphics_queue);
		log_queue(RHIQueueType::Compute, m_compute_queue);
		log_queue(RHIQueueType::Transfer, m_transfer_queue);
		m_pipeline_layout_manager = trx_new VulkanPipelineLayoutManager();
		m_query_pool_manager      = trx_new VulkanQueryPoolManager();
		m_descriptor_heap         = trx_new VulkanDescriptorHeap();

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
			m_timeline = vk::verify(m_device.createSemaphore(create_info));
		}
	}

	VulkanAPI::~VulkanAPI()
	{
		wait();

		for (VulkanThreadLocal* local : m_thread_locals)
		{
			trx_delete local;
		}

		trx_delete m_pipeline_layout_manager;
		for (RHIQueue* queue : m_queues)
			trx_delete static_cast<VulkanQueue*>(queue);
		m_queues.clear();
		trx_delete m_query_pool_manager;
		trx_delete m_descriptor_heap;

		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;

		m_device.destroySemaphore(m_timeline);

		m_device.destroy();
		if (m_debug_messenger)
			m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger);
		m_instance.destroy();
	}

	vk::PresentModeKHR VulkanAPI::present_mode_of(RHIPresentMode mode, vk::SurfaceKHR surface)
	{
		auto modes = vk::verify(m_physical_device.getSurfacePresentModesKHR(surface));
		trinex_verify_msg(!modes.empty(), "Failed to find present mode!");

		vk::PresentModeKHR required = VulkanEnums::present_mode_of(mode);

		for (vk::PresentModeKHR supported : modes)
		{
			if (supported == required)
				return supported;
		}

		return vk::PresentModeKHR::eFifo;
	}

	vk::Extent2D VulkanAPI::surface_size(const vk::SurfaceKHR& surface) const
	{
		return vk::verify(m_physical_device.getSurfaceCapabilitiesKHR(surface)).currentExtent;
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

	VulkanAPI& VulkanAPI::wait()
	{
		vk::verify(m_device.waitIdle());
		return *this;
	}

	RHIQueue* VulkanAPI::queue(RHIQueueType type, RHISwapchain* swapchain)
	{
		switch (type)
		{
			case RHIQueueType::Graphics: return m_graphics_queue;
			case RHIQueueType::Compute: return m_compute_queue;
			case RHIQueueType::Transfer: return m_transfer_queue;
			case RHIQueueType::Present: return m_graphics_queue;
			default: return nullptr;
		}
	}

	RHIArrayView<RHIQueue*> VulkanAPI::queues() const
	{
		return {m_queues.data(), m_queues.size()};
	}
} // namespace Trinex
