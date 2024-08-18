#include <VkBootstrap.h>

#include <Core/struct.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/texture.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_pipeline.hpp>
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
#if ENABLE_VALIDATION_LAYERS
	const Vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation",
	};
#endif

	VulkanAPI* VulkanAPI::m_vulkan = nullptr;

	implement_struct(Engine::RHI, VULKAN, ).push([]() {
		Struct::static_find("Engine::RHI::VULKAN", true)->struct_constructor([]() -> void* {
			if (VulkanAPI::m_vulkan == nullptr)
			{
				VulkanAPI::m_vulkan						  = new VulkanAPI();
				VulkanAPI::m_vulkan->info.name			  = "Vulkan";
				VulkanAPI::m_vulkan->info.struct_instance = Struct::static_find("Engine::RHI::VULKAN", true);
			}
			return VulkanAPI::m_vulkan;
		});
	});

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

		for (VulkanUniformBuffer* buffer : m_uniform_buffer)
		{
			delete buffer;
		}

		m_uniform_buffer.clear();
		VulkanDescriptorPoolManager::release_all();

		DESTROY_CALL(destroyDescriptorPool, m_imgui_descriptor_pool);
		m_device.destroyCommandPool(m_command_pool);
		m_device.destroy();
		vkb::destroy_instance(m_instance);
	}

	vk::PresentModeKHR VulkanAPI::present_mode_of(bool vsync)
	{
		return vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eImmediate;
	}

	bool VulkanAPI::vsync_from_present_mode(vk::PresentModeKHR present_mode)
	{
		return present_mode == vk::PresentModeKHR::eFifo ? true : false;
	}

	vk::Device* vulkan_device()
	{
		return &API->m_device;
	}

	vk::CommandPool* vulkan_command_pool()
	{
		return &API->m_command_pool;
	}

	///////////////////////////////// INITIALIZATION /////////////////////////////////
#if ENABLE_VALIDATION_LAYERS
	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
														 VkDebugUtilsMessageTypeFlagsEXT message_type,
														 const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
														 void* pUserData)
	{
#define has_bit(bit) ((message_severity & bit) == bit)

		if (has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) ||
			has_bit(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT))
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

	static bool is_available_swapchain_images_count(uint32_t count)
	{
		auto& m_surface_capabilities = API->m_surface_capabilities;
		return (m_surface_capabilities.maxImageCount == 0 || m_surface_capabilities.maxImageCount >= count) &&
			   m_surface_capabilities.minImageCount <= count;
	}

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

		phys_device_selector.set_surface(static_cast<VkSurfaceKHR>(API->m_surface));

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

		new_features.samplerAnisotropy	= features.samplerAnisotropy;
		new_features.fillModeNonSolid	= features.fillModeNonSolid;
		new_features.wideLines			= features.wideLines;
		new_features.tessellationShader = features.tessellationShader;
		new_features.geometryShader		= features.geometryShader;

		return new_features;
	}

	VulkanAPI& VulkanAPI::initialize(Window* window)
	{
		m_window = window;

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
				.request_validation_layers()
				.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
#else
		instance_builder.add_validation_disable(VK_VALIDATION_CHECK_ALL_EXT);
		instance_builder.add_validation_feature_disable(VK_VALIDATION_FEATURE_DISABLE_ALL_EXT);
#endif

		auto instance_ret = instance_builder.build();

		if (!instance_ret)
		{
			auto message = instance_ret.error().message();
			vulkan_error_log("Vulkan", "%s", message.c_str());
			throw EngineException(message);
		}

		m_instance = instance_ret.value();

		m_surface = create_surface(window);


		// Initialize physical device
		auto selected_device = initialize_physical_device();
		m_physical_device	 = vk::PhysicalDevice(selected_device.physical_device);

		m_properties		   = m_physical_device.getProperties();
		m_features			   = filter_features(m_physical_device.getFeatures());
		m_surface_capabilities = m_physical_device.getSurfaceCapabilitiesKHR(m_surface);
		info.renderer		   = m_properties.deviceName.data();
		selected_device.enable_features_if_present(m_features);

		info_log("Vulkan", "Selected GPU '%s'", info.renderer.c_str());


		// Initialize device
		m_bootstrap_device = build_device(selected_device);
		m_device		   = vk::Device(m_bootstrap_device.device);

		auto index_1		= m_bootstrap_device.get_queue_index(vkb::QueueType::graphics);
		auto index_2		= m_bootstrap_device.get_queue_index(vkb::QueueType::present);
		auto graphics_queue = m_bootstrap_device.get_queue(vkb::QueueType::graphics);
		auto present_queue	= m_bootstrap_device.get_queue(vkb::QueueType::present);

		if (!index_1.has_value() || !index_2.has_value() || !graphics_queue.has_value() || !present_queue.has_value())
		{
			throw std::runtime_error("Failed to init queues");
		}

		m_graphics_queue_index = index_1.value();
		m_present_queue_index  = index_2.value();

		m_graphics_queue = vk::Queue(graphics_queue.value());
		m_present_queue	 = vk::Queue(present_queue.value());


		initialize_pfn();
		enable_dynamic_states();
		create_command_pool();

		if (is_available_swapchain_images_count(VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT))
		{
			m_framebuffers_count = VULKAN_DESIRED_SWAPCHAIN_IMAGES_COUNT;
		}
		else if (is_available_swapchain_images_count(VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT))
		{
			m_framebuffers_count = VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT;
		}
		else if (m_surface_capabilities.minImageCount >= VULKAN_MIN_SWAPCHAIN_IMAGES_COUNT)
		{
			m_framebuffers_count = m_surface_capabilities.minImageCount;
		}
		else
		{
			throw EngineException("Vulkan requires a minimum of 2 images for Swapchain");
		}

		m_uniform_buffer.resize(m_framebuffers_count);

		for (VulkanUniformBuffer*& buffer : m_uniform_buffer)
		{
			buffer = new VulkanUniformBuffer();
		}
		return *this;
	}

	void* VulkanAPI::context()
	{
		return nullptr;
	}

	void VulkanAPI::enable_dynamic_states()
	{
		m_dynamic_states = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor,
		};
	}

	void VulkanAPI::initialize_pfn()
	{
		pfn.vkCmdBeginDebugUtilsLabelEXT =
				(PFN_vkCmdBeginDebugUtilsLabelEXT) vkGetDeviceProcAddr(m_device, "vkCmdBeginDebugUtilsLabelEXT");
		pfn.vkCmdEndDebugUtilsLabelEXT =
				(PFN_vkCmdEndDebugUtilsLabelEXT) vkGetDeviceProcAddr(m_device, "vkCmdEndDebugUtilsLabelEXT");
	}


	vk::SurfaceKHR VulkanAPI::create_surface(Window* window)
	{
		extern vk::SurfaceKHR create_vulkan_surface(void* native_window, vk::Instance instance);
		return create_vulkan_surface(window->native_window(), m_instance.instance);
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


	vk::Extent2D VulkanAPI::surface_size() const
	{
		return surface_size(m_surface);
	}

	vk::Extent2D VulkanAPI::surface_size(const vk::SurfaceKHR& surface) const
	{
		return m_physical_device.getSurfaceCapabilitiesKHR(surface).currentExtent;
	}

	bool VulkanAPI::has_stencil_component(vk::Format format)
	{
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	VulkanAPI& VulkanAPI::create_image(VulkanTexture* texture, vk::ImageTiling tiling, vk::ImageCreateFlags flags,
									   vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image,
									   vk::DeviceMemory& image_memory, uint32_t layers)
	{

		vk::ImageCreateInfo image_info(flags, vk::ImageType::e2D, texture->format(),
									   vk::Extent3D(texture->size().x, texture->size().y, 1), texture->mipmap_count(),
									   layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive);

		image									   = API->m_device.createImage(image_info);
		vk::MemoryRequirements memory_requirements = API->m_device.getImageMemoryRequirements(image);
		auto memory_type =
				API->find_memory_type(memory_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vk::MemoryAllocateInfo alloc_info(memory_requirements.size, memory_type);
		image_memory = API->m_device.allocateMemory(alloc_info);
		API->m_device.bindImageMemory(image, image_memory, 0);
		return *this;
	}

	void VulkanAPI::create_command_pool()
	{
		m_command_pool = m_device.createCommandPool(
				vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_graphics_queue_index));
	}


	VulkanAPI& VulkanAPI::begin_render()
	{
		++m_current_frame;
		m_current_buffer = m_current_frame % m_framebuffers_count;

		uniform_buffer()->reset();
		return *this;
	}

	VulkanAPI& VulkanAPI::end_render()
	{
		if (m_state.m_current_viewport)
		{
			m_state.m_current_viewport->end_render();
		}
		return *this;
	}

	uint32_t VulkanAPI::find_memory_type(uint32_t type_filter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties mem_properties = m_physical_device.getMemoryProperties();

		for (uint32_t i = 0; (1u << i) <= type_filter && i < mem_properties.memoryTypeCount; i++)
		{
			if ((type_filter & (1u << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties))
			{
				return i;
			}
		}

		throw std::runtime_error("VulkanAPI: Failed to find suitable memory type!");
	}

	VulkanAPI& VulkanAPI::create_buffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
										vk::MemoryPropertyFlags properties, vk::Buffer& buffer,
										vk::DeviceMemory& buffer_memory)
	{
		vk::BufferCreateInfo buffer_info({}, size, usage, vk::SharingMode::eExclusive);
		buffer = m_device.createBuffer(buffer_info);

		vk::MemoryRequirements mem_requirements = m_device.getBufferMemoryRequirements(buffer);
		vk::MemoryAllocateInfo alloc_info(mem_requirements.size,
										  find_memory_type(mem_requirements.memoryTypeBits, properties));


		buffer_memory = m_device.allocateMemory(alloc_info);
		m_device.bindBufferMemory(buffer, buffer_memory, 0);
		return *this;
	}

	vk::CommandBuffer VulkanAPI::begin_single_time_command_buffer()
	{
		vk::CommandBufferAllocateInfo alloc_info(m_command_pool, vk::CommandBufferLevel::ePrimary, 1);
		vk::CommandBuffer command_buffer = m_device.allocateCommandBuffers(alloc_info).front();
		vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		command_buffer.begin(begin_info);
		return command_buffer;
	}

	VulkanAPI& VulkanAPI::end_single_time_command_buffer(const vk::CommandBuffer& command_buffer)
	{
		command_buffer.end();
		vk::SubmitInfo submit_info({}, {}, command_buffer);
		m_graphics_queue.submit(submit_info, {});
		m_graphics_queue.waitIdle();
		m_device.freeCommandBuffers(m_command_pool, command_buffer);
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
		m_graphics_queue.waitIdle();
		m_present_queue.waitIdle();

		return *this;
	}

	VulkanAPI& VulkanAPI::prepare_draw()
	{
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
		prepare_draw().current_command_buffer_handle().drawIndexed(indices_count, instances, indices_offset,
																   vertices_offset, 0);
		return *this;
	}

	VulkanUniformBuffer* VulkanAPI::uniform_buffer() const
	{
		return m_uniform_buffer[m_current_buffer];
	}

	VulkanAPI& VulkanAPI::push_debug_stage(const char* stage, const Color& color)
	{
		if (pfn.vkCmdBeginDebugUtilsLabelEXT)
		{
			VkDebugUtilsLabelEXT label_info = {};
			label_info.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			label_info.pLabelName			= stage;
			label_info.color[0]				= color.r;
			label_info.color[1]				= color.g;
			label_info.color[2]				= color.b;
			label_info.color[3]				= color.a;

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
