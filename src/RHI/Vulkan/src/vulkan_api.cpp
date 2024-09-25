#include <VkBootstrap.h>

#include <Core/profiler.hpp>
#include <Core/struct.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
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

#define VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT 2
#define VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT 3

namespace Engine
{
	VulkanAPI* VulkanAPI::m_vulkan = nullptr;

	using VULKAN = VulkanAPI;

	implement_struct_default_init(Engine::RHI, VULKAN);

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
		return modes[0];
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

		phys_device_selector.allow_any_gpu_device_type(false);
#if USE_INTEGRATED_GPU
		phys_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::integrated);
#else
		phys_device_selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);
#endif
		phys_device_selector.defer_surface_initialization();

		auto selected_device = phys_device_selector.select();
		if (!selected_device.has_value())
		{
			auto msg = selected_device.error().message();
			vulkan_error_log("Vulkan", "%s", msg.c_str());
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

	VulkanAPI& VulkanAPI::initialize(Window* window)
	{
		vkb::InstanceBuilder instance_builder;
		instance_builder.require_api_version(VK_API_VERSION_1_0);

		Vector<String> required_extensions;
		extern void load_required_extensions(void* native_window, Vector<String>& required_extensions);
		load_required_extensions(window->native_window(), required_extensions);

		for (auto& extension : required_extensions)
		{
			vulkan_info_log("VulkanAPI", "Enable extention %s", extension.c_str());
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
			vulkan_error_log("Vulkan", "%s", message.c_str());
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
			VmaVulkanFunctions vulkan_functions = {};
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
		auto vp = m_state.m_current_viewport;
		auto rt = m_state.m_render_target;
		if (vp == nullptr || rt == nullptr)
		{
			return VulkanViewportMode::Undefined;
		}
		if (vp->is_window_viewport() && vp->render_target() == rt)
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

	VulkanAPI& VulkanAPI::begin_render_pass(bool lock)
	{
		trinex_profile_cpu();
		if (m_state.m_next_render_target)
		{
			m_state.m_render_target      = m_state.m_next_render_target;
			m_state.m_next_render_target = nullptr;
		}

		if (lock)
			m_state.m_render_target->lock_surfaces();

		m_state.m_render_pass = m_state.m_render_target->m_render_pass;
		current_command_buffer()->begin_render_pass(m_state.m_render_target);
		return *this;
	}

	VulkanAPI& VulkanAPI::end_render_pass(bool unlock)
	{
		trinex_profile_cpu();
		current_command_buffer()->end_render_pass();

		if (unlock)
			m_state.m_render_target->unlock_surfaces();

		m_state.m_render_pass = nullptr;
		return *this;
	}

	VulkanAPI& VulkanAPI::begin_render()
	{
		return *this;
	}

	VulkanAPI& VulkanAPI::end_render()
	{
		if (m_state.m_current_viewport)
		{
			m_state.m_current_viewport->end_render();
		}

		m_stagging_manager->update();
		return *this;
	}

	vk::CommandBuffer VulkanAPI::begin_single_time_command_buffer()
	{
		vk::CommandBufferAllocateInfo alloc_info(m_cmd_manager->m_pool.m_pool, vk::CommandBufferLevel::ePrimary, 1);
		vk::CommandBuffer command_buffer = m_device.allocateCommandBuffers(alloc_info).front();
		vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		command_buffer.begin(begin_info);
		return command_buffer;
	}

	VulkanAPI& VulkanAPI::end_single_time_command_buffer(const vk::CommandBuffer& command_buffer)
	{
		command_buffer.end();
		vk::SubmitInfo submit_info({}, {}, command_buffer);
		m_graphics_queue->submit(submit_info);
		m_graphics_queue->wait_idle();
		m_device.freeCommandBuffers(m_cmd_manager->m_pool.m_pool, command_buffer);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_buffer(vk::Buffer src_buffer, vk::Buffer dst_buffer, vk::DeviceSize size,
	                                  vk::DeviceSize src_offset, vk::DeviceSize dst_offset)
	{
		auto command_buffer = begin_single_time_command_buffer();
		vk::BufferCopy copy_region(src_offset, dst_offset, size);
		command_buffer.copyBuffer(src_buffer, dst_buffer, copy_region);
		return end_single_time_command_buffer(command_buffer);
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
		trinex_profile_cpu();

		trinex_check(m_state.m_pipeline, "Pipeline can't be nullptr");
		trinex_check(m_state.m_render_target || m_state.m_next_render_target, "Render target can't be nullptr");

		auto cmd                    = current_command_buffer();
		bool is_render_target_dirty = m_state.m_next_render_target;

		if (is_render_target_dirty && cmd->is_inside_render_pass())
			end_render_pass();

		if (cmd->is_outside_render_pass())
			begin_render_pass();

		if (is_render_target_dirty)
		{
			if (find_current_viewport_mode() != m_state.m_viewport_mode)
			{
				viewport(m_state.m_viewport);
				scissor(m_state.m_scissor);
			}
		}

		uniform_buffer()->bind();
		m_state.m_pipeline->bind_descriptor_set();
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

	VulkanAPI& VulkanAPI::push_debug_stage(const char* stage, const Color& color)
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
