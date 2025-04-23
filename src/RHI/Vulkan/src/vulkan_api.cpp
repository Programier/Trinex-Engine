#include <VkBootstrap.h>

#include <Core/engine_loading_controllers.hpp>
#include <Core/exception.hpp>
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
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>
#include <vulkan_uniform_buffer.hpp>
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
			VulkanAPI::m_vulkan                       = new VulkanAPI();
			VulkanAPI::m_vulkan->info.name            = "Vulkan";
			VulkanAPI::m_vulkan->info.struct_instance = static_struct_instance();
		}
		return VulkanAPI::m_vulkan;
	}

	void VulkanAPI::static_destructor(VulkanAPI* vulkan)
	{
		if (vulkan == m_vulkan)
		{
			delete vulkan;
			m_vulkan = nullptr;
		}
	}

	VulkanAPI::VulkanAPI()
	{
		m_device_extensions = {
		        {VK_KHR_MAINTENANCE1_EXTENSION_NAME, true, false},
		        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, false},
		        {VK_EXT_INDEX_TYPE_UINT8_EXTENSION_NAME, true, false},
		        {VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, false, false},
		};
	}

	VulkanAPI::~VulkanAPI()
	{
		wait_idle();

		VulkanRenderPass::destroy_all();

		delete m_cmd_manager;
		delete m_stagging_manager;
		delete m_graphics_queue;

		if (m_present_queue != m_graphics_queue)
		{
			delete m_present_queue;
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
#if ENABLE_VALIDATION_LAYERS
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
#endif

	static vkb::PhysicalDevice initialize_physical_device()
	{
		vkb::PhysicalDeviceSelector phys_device_selector(API->m_instance);

		for (VulkanExtention& extension : API->m_device_extensions)
		{
			if (extension.required)
			{
				phys_device_selector.add_required_extension(extension.name);
				extension.enabled = true;
			}
		}

		phys_device_selector.defer_surface_initialization();

		phys_device_selector.allow_any_gpu_device_type(VulkanConfig::allow_any_gpu_type);
		phys_device_selector.require_present(VulkanConfig::require_present);
		phys_device_selector.prefer_gpu_device_type(VulkanConfig::device_type);

		if (VulkanConfig::require_dedicated_transfer_queue)
			phys_device_selector.require_dedicated_transfer_queue();
		if (VulkanConfig::require_dedicated_compute_queue)
			phys_device_selector.require_dedicated_compute_queue();
		if (VulkanConfig::require_separate_transfer_queue)
			phys_device_selector.require_separate_transfer_queue();
		if (VulkanConfig::require_separate_compute_queue)
			phys_device_selector.require_separate_compute_queue();

		auto selected_device = phys_device_selector.select();
		if (!selected_device.has_value())
		{
			auto msg = selected_device.error().message();
			error_log("Vulkan", "%s", msg.c_str());
			throw std::runtime_error(msg);
		}

		auto device = selected_device.value();

		for (VulkanExtention& extension : API->m_device_extensions)
		{
			if (!extension.required)
			{
				extension.enabled = device.enable_extension_if_present(extension.name);
			}
		}

		return device;
	}


	static vkb::Device build_device(vkb::PhysicalDevice& physical_device)
	{
		vkb::DeviceBuilder builder(physical_device);

		vk::PhysicalDeviceIndexTypeUint8FeaturesEXT idx_byte_feature(VK_TRUE);
		builder.add_pNext(&idx_byte_feature);
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

		new_features.samplerAnisotropy  = features.samplerAnisotropy;
		new_features.fillModeNonSolid   = features.fillModeNonSolid;
		new_features.wideLines          = features.wideLines;
		new_features.tessellationShader = features.tessellationShader;
		new_features.geometryShader     = features.geometryShader;

		return new_features;
	}

	static void initialize_color_formats(ColorFormat color_format)
	{
		using Capabilities = ColorFormat::Capabilities;
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

		if (Settings::debug_shaders)
			instance_builder.require_api_version(VK_API_VERSION_1_3);
		else
			instance_builder.require_api_version(VK_API_VERSION_1_0);

		Vector<String> required_extensions;
		extern void load_required_extensions(void* native_window, Vector<String>& required_extensions);
		load_required_extensions(window->native_window(), required_extensions);

		for (auto& extension : required_extensions)
		{
			info_log("VulkanAPI", "Enable extention %s", extension.c_str());
			instance_builder.enable_extension(extension.c_str());
		}

#if ENABLE_VALIDATION_LAYERS
		instance_builder.set_debug_callback(debug_callback)
		        .request_validation_layers(true)
		        .enable_validation_layers(true)
		        .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
#else
		instance_builder.enable_validation_layers(false).request_validation_layers(false);
#endif

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

		ColorFormat::static_foreach(initialize_color_formats);

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

		m_graphics_queue = new VulkanQueue(graphics_queue.value(), graphics_queue_index.value());

		initialize_pfn();

		m_cmd_manager      = new VulkanCommandBufferManager();
		m_stagging_manager = new VulkanStaggingBufferManager();


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
		vk::SurfaceKHR surface = create_vulkan_surface(window->native_window(), m_instance.instance);
		setup_present_queue(surface);
		return surface;
	}

	VulkanAPI& VulkanAPI::setup_present_queue(vk::SurfaceKHR surface)
	{
		if (m_present_queue)
			return *this;

		m_present_queue = m_graphics_queue;
		return *this;
	}

	VulkanViewportMode VulkanAPI::find_current_viewport_mode()
	{
		auto rt = m_state.render_target();

		if (rt == nullptr)
		{
			return VulkanViewportMode::Undefined;
		}

		if (rt->is_swapchain_render_target())
			return VulkanViewportMode::Flipped;
		return VulkanViewportMode::Normal;
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
		trinex_profile_cpu_n("VulkanAPI::begin_render_pass");
		auto cmd = current_command_buffer();

		if (m_state.m_next_render_target)
		{
			m_state.m_render_target      = m_state.m_next_render_target;
			m_state.m_next_render_target = nullptr;
		}

		m_state.m_render_target->lock_surfaces();
		m_state.m_render_pass = m_state.m_render_target->m_render_pass;
		cmd->begin_render_pass(m_state.m_render_target);
		return cmd;
	}

	VulkanCommandBuffer* VulkanAPI::end_render_pass()
	{
		auto cmd = current_command_buffer();
		if (m_state.m_render_pass)
		{
			trinex_profile_cpu_n("VulkanAPI::end_render_pass");
			cmd->end_render_pass();
			m_state.m_render_pass = nullptr;
		}
		return cmd;
	}

	bool VulkanAPI::is_format_supported(vk::Format format, vk::FormatFeatureFlagBits flags, bool optimal)
	{
		vk::FormatProperties properties            = m_physical_device.getFormatProperties(format);
		const vk::FormatFeatureFlags feature_flags = optimal ? properties.optimalTilingFeatures : properties.linearTilingFeatures;
		return (feature_flags & flags) == flags;
	}

	VulkanAPI& VulkanAPI::submit()
	{
		m_stagging_manager->update();

		if (m_cmd_manager->has_pending_active_cmd_buffer())
		{
			m_cmd_manager->submit_active_cmd_buffer();
		}

		API->m_state.reset();
		return *this;
	}

	VulkanAPI& VulkanAPI::wait_idle()
	{
		m_device.waitIdle();
		m_graphics_queue->wait_idle();
		m_present_queue->wait_idle();

		return *this;
	}

	VulkanAPI& VulkanAPI::prepare_draw()
	{
		trinex_profile_cpu_n("VulkanAPI::prepare_draw");

		trinex_check(m_state.m_pipeline, "Pipeline can't be nullptr");
		trinex_check(m_state.render_target(), "Render target can't be nullptr");

		auto cmd                    = current_command_buffer();
		bool is_render_target_dirty = m_state.m_next_render_target != nullptr;

		if (is_render_target_dirty)
		{
			if (cmd->is_inside_render_pass())
				end_render_pass();

			if (find_current_viewport_mode() != m_state.m_viewport_mode)
			{
				viewport(m_state.m_viewport);
				scissor(m_state.m_scissor);
			}
		}

		if (cmd->is_outside_render_pass())
			begin_render_pass();

		uniform_buffer_manager()->bind();
		m_state.m_pipeline->bind_descriptor_set(vk::PipelineBindPoint::eGraphics);
		return *this;
	}

	VulkanAPI& VulkanAPI::prepare_dispatch()
	{
		trinex_profile_cpu_n("VulkanAPI::prepare_dispatch");
		trinex_check(m_state.m_pipeline, "Pipeline can't be nullptr");

		uniform_buffer_manager()->bind();
		m_state.m_pipeline->bind_descriptor_set(vk::PipelineBindPoint::eCompute);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw(size_t vertex_count, size_t vertices_offset)
	{
		prepare_draw().current_command_buffer_handle().draw(vertex_count, 1, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_indexed(size_t indices, size_t offset, size_t vertices_offset)
	{
		prepare_draw().current_command_buffer_handle().drawIndexed(indices, 1, offset, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
	{
		prepare_draw().current_command_buffer_handle().draw(vertex_count, instances, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
	                                             size_t instances)
	{
		prepare_draw().current_command_buffer_handle().drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanAPI& VulkanAPI::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
	{
		prepare_dispatch().current_command_buffer_handle().dispatch(group_x, group_y, group_z);
		return *this;
	}

	VulkanAPI& VulkanAPI::push_debug_stage(const char* stage, const LinearColor& color)
	{
		if (pfn.vkCmdBeginDebugUtilsLabelEXT)
		{
			VkDebugUtilsLabelEXT label_info = {};
			label_info.sType                = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label_info.pLabelName           = stage;
			label_info.color[0]             = color.r;
			label_info.color[1]             = color.g;
			label_info.color[2]             = color.b;
			label_info.color[3]             = color.a;

			pfn.vkCmdBeginDebugUtilsLabelEXT(current_command_buffer_handle(), &label_info);
		}

		return *this;
	}

	VulkanAPI& VulkanAPI::pop_debug_stage()
	{
		if (pfn.vkCmdEndDebugUtilsLabelEXT)
		{
			pfn.vkCmdEndDebugUtilsLabelEXT(current_command_buffer_handle());
		}
		return *this;
	}
}// namespace Engine
