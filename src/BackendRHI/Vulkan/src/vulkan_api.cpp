#include <VkBootstrap.h>

#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/struct.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_config.hpp>
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
#include <vulkan_viewport.hpp>

namespace Engine
{
	VulkanAPI* VulkanAPI::m_vulkan = nullptr;

	namespace TRINEX_RHI
	{
		using VULKAN = VulkanAPI;
	}

	trinex_implement_struct_default_init(Engine::TRINEX_RHI::VULKAN, 0);

	VulkanAPI* VulkanAPI::static_constructor()
	{
		if (VulkanAPI::m_vulkan == nullptr)
		{
			VulkanAPI::m_vulkan                       = allocate<VulkanAPI>();
			VulkanAPI::m_vulkan->info.name            = "Vulkan";
			VulkanAPI::m_vulkan->info.struct_instance = static_struct_instance();
		}
		return VulkanAPI::m_vulkan;
	}

	void VulkanAPI::static_destructor(VulkanAPI* vulkan)
	{
		if (vulkan == m_vulkan)
		{
			release(vulkan);
			m_vulkan = nullptr;
		}
	}

	VulkanAPI::VulkanAPI()
	{
		m_device_extensions = {
		        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, false},
		        {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false, false},
		        {VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, false, false},
		        {VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME, false, false},
		};
	}

	VulkanAPI::~VulkanAPI()
	{
		wait_idle();

		VulkanRenderPass::destroy_all();

		release(m_stagging_manager);
		release(m_state_manager);
		release(m_cmd_manager);
		release(m_graphics_queue);
		release(m_descriptor_set_allocator);
		release(m_query_pool_manager);

		for (auto& [hash, layout] : m_pipeline_layouts)
		{
			release(layout);
		}

		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;

		m_device.destroy();
		vkb::destroy_instance(m_instance);
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

	///////////////////////////////// INITIALIZATION /////////////////////////////////
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

		for (const VulkanExtention& extension : API->m_device_extensions)
		{
			if (extension.required)
			{
				phys_device_selector.add_required_extension(extension.name.data());
				extension.enabled = true;
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

		for (const VulkanExtention& extension : API->m_device_extensions)
		{
			if (!extension.required)
			{
				extension.enabled = device.enable_extension_if_present(extension.name.data());
			}
		}

		return device;
	}

	static vkb::Device build_device(vkb::PhysicalDevice& physical_device)
	{
		vkb::DeviceBuilder builder(physical_device);

		vk::PhysicalDeviceCustomBorderColorFeaturesEXT custom_border(vk::True, vk::False);
		vk::PhysicalDeviceVulkan11Features vk11_features;

		vk11_features.setShaderDrawParameters(vk::True);

		builder.add_pNext(&vk11_features);

		if (API->is_extension_enabled(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
			builder.add_pNext(&custom_border);

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

	static void initialize_color_formats(RHIColorFormat color_format)
	{
		using Capabilities = RHIColorFormat::Capabilities;
		vk::Format format  = VulkanEnums::format_of(color_format);
		auto features      = API->m_physical_device.getFormatProperties(format).optimalTilingFeatures;

		Capabilities capabilities = 0;

		if (features & vk::FormatFeatureFlagBits::eSampledImage)
			capabilities |= Capabilities(Capabilities::TextureSample | Capabilities::TextureGather | Capabilities::Texture1D |
			                             Capabilities::Texture2D | Capabilities::Texture3D | Capabilities::TextureCube |
			                             Capabilities::TextureMipmaps);

		if (features & vk::FormatFeatureFlagBits::eStorageImage)
			capabilities |= Capabilities(Capabilities::TextureStore | Capabilities::TextureLoad);

		if (features & vk::FormatFeatureFlagBits::eStorageImageAtomic)
			capabilities |= Capabilities::TextureAtomics;

		if (features & vk::FormatFeatureFlagBits::eColorAttachment)
			capabilities |= Capabilities::RenderTarget;

		if (features & vk::FormatFeatureFlagBits::eColorAttachmentBlend)
			capabilities |= Capabilities::TextureBlendable;

		if (features & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
			capabilities |= Capabilities::DepthStencil;

		color_format.add_capabilities(capabilities);
	}

	VulkanAPI& VulkanAPI::initialize(Window* window)
	{
		vkb::InstanceBuilder instance_builder;
		instance_builder.require_api_version(VK_API_VERSION_1_1);

		Vector<String> required_extensions;
		extern void load_required_extensions(void* native_window, Vector<String>& required_extensions);
		load_required_extensions(window->native_window(), required_extensions);

		for (auto& extension : required_extensions)
		{
			info_log("VulkanAPI", "Enable extention %s", extension.c_str());
			instance_builder.enable_extension(extension.c_str());
		}

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

		RHIColorFormat::static_foreach(initialize_color_formats);

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

		m_graphics_queue = allocate<VulkanQueue>(graphics_queue.value(), graphics_queue_index.value());

		initialize_pfn();

		m_cmd_manager              = allocate<VulkanCommandBufferManager>();
		m_stagging_manager         = allocate<VulkanStaggingBufferManager>();
		m_state_manager            = allocate<VulkanStateManager>();
		m_descriptor_set_allocator = allocate<VulkanDescriptorSetAllocator>();
		m_query_pool_manager       = allocate<VulkanQueryPoolManager>();


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
			vmaCreateAllocator(&allocator_info, &m_allocator);
		}


		return *this;
	}

	void* VulkanAPI::context()
	{
		return nullptr;
	}

	void VulkanAPI::initialize_pfn()
	{
		auto load = [this]<typename T>(T& func, const char* name) {
			func = reinterpret_cast<T>(vkGetDeviceProcAddr(m_device, name));
		};

		load(pfn.vkCmdBeginDebugUtilsLabelEXT, "vkCmdBeginDebugUtilsLabelEXT");
		load(pfn.vkCmdEndDebugUtilsLabelEXT, "vkCmdEndDebugUtilsLabelEXT");
		load(pfn.vkGetBufferMemoryRequirements2KHR, "vkGetBufferMemoryRequirements2KHR");
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

	VulkanCommandBuffer* VulkanAPI::begin_render_pass()
	{
		return m_state_manager->begin_render_pass();
	}

	VulkanCommandBuffer* VulkanAPI::end_render_pass()
	{
		return m_state_manager->end_render_pass();
	}

	bool VulkanAPI::is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal)
	{
		vk::FormatProperties properties            = m_physical_device.getFormatProperties(format);
		const vk::FormatFeatureFlags feature_flags = optimal ? properties.optimalTilingFeatures : properties.linearTilingFeatures;
		return (feature_flags & flags) == flags;
	}

	bool VulkanAPI::is_extension_enabled(const char* extension)
	{
		VulkanExtention ext;
		ext.name = extension;

		auto it = m_device_extensions.find(ext);
		if (it == m_device_extensions.end())
			return false;

		return it->enabled;
	}

	VulkanAPI& VulkanAPI::submit()
	{
		m_stagging_manager->update();
		m_cmd_manager->submit();
		m_state_manager->submit();
		return *this;
	}

	VulkanAPI& VulkanAPI::wait_idle()
	{
		m_device.waitIdle();
		m_graphics_queue->queue().waitIdle();
		return *this;
	}

	VulkanAPI& VulkanAPI::draw(size_t vertex_count, size_t vertices_offset)
	{
		m_state_manager->flush_graphics()->draw(vertex_count, 1, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_indexed(size_t indices, size_t offset, size_t vertices_offset)
	{
		m_state_manager->flush_graphics()->drawIndexed(indices, 1, offset, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		m_state_manager->flush_graphics()->draw(vertex_count, instances, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                             size_t instances)
	{
		m_state_manager->flush_graphics()->drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		m_state_manager->flush_compute()->dispatch(group_x, group_y, group_z);
		return *this;
	}

	VulkanAPI& VulkanAPI::push_debug_stage(const char* stage)
	{
		if (pfn.vkCmdBeginDebugUtilsLabelEXT)
		{
			VkDebugUtilsLabelEXT label_info = {};
			label_info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label_info.pLabelName           = stage;
			label_info.color[0]             = 1.f;
			label_info.color[1]             = 1.f;
			label_info.color[2]             = 1.f;
			label_info.color[3]             = 1.f;

			pfn.vkCmdBeginDebugUtilsLabelEXT(*current_command_buffer(), &label_info);
		}
		return *this;
	}

	VulkanAPI& VulkanAPI::pop_debug_stage()
	{
		if (pfn.vkCmdEndDebugUtilsLabelEXT)
		{
			pfn.vkCmdEndDebugUtilsLabelEXT(*current_command_buffer());
		}
		return *this;
	}

	VulkanAPI& VulkanAPI::viewport(const RHIViewport& viewport)
	{
		vk::Viewport vulkan_viewport;
		vulkan_viewport.setWidth(viewport.size.x);
		vulkan_viewport.setHeight(viewport.size.y);
		vulkan_viewport.setX(viewport.pos.x);
		vulkan_viewport.setY(viewport.pos.y);
		vulkan_viewport.setMinDepth(viewport.min_depth);
		vulkan_viewport.setMaxDepth(viewport.max_depth);
		current_command_buffer()->setViewport(0, vulkan_viewport);
		return *this;
	}

	VulkanAPI& VulkanAPI::scissor(const RHIScissors& unnormalized_scissors)
	{
		vk::Rect2D vulkan_scissor;

		static auto normalize = [](RHIScissors rect) {
			if (rect.pos.x < 0)
			{
				rect.size.x -= rect.pos.x;
				rect.pos.x = 0;
			}

			if (rect.pos.y < 0)
			{
				rect.size.y -= rect.pos.y;
				rect.pos.y = 0;
			}
			return rect;
		};

		RHIScissors scissors = normalize(unnormalized_scissors);

		vulkan_scissor.offset.setX(scissors.pos.x);
		vulkan_scissor.offset.setY(scissors.pos.y);
		vulkan_scissor.extent.setWidth(scissors.size.x);
		vulkan_scissor.extent.setHeight(scissors.size.y);

		current_command_buffer()->setScissor(0, vulkan_scissor);
		return *this;
	}

	void trinex_vulkan_deferred_destroy(RHI_Object* object)
	{
		API->current_command_buffer()->destroy_object(object);
	}
}// namespace Engine
