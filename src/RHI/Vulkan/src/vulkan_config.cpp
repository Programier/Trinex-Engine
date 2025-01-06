#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/enum.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <VkBootstrap.h>
#include <vulkan_config.hpp>

namespace vkb
{
    using DeviceType = PreferredDeviceType;
}

implement_enum(vkb::DeviceType, vkb::DeviceType::other, vkb::DeviceType::integrated, vkb::DeviceType::discrete,
               vkb::DeviceType::virtual_gpu, vkb::DeviceType::cpu);

namespace Engine::VulkanConfig
{
	vkb::PreferredDeviceType device_type  = vkb::PreferredDeviceType::discrete;
	bool enable_validation                = false;
	bool allow_any_gpu_type               = false;
	bool require_present                  = true;
	bool require_dedicated_transfer_queue = false;
	bool require_dedicated_compute_queue  = false;
	bool require_separate_transfer_queue  = false;
	bool require_separate_compute_queue   = false;
	unsigned int required_mem_size        = 0;
	unsigned int desired_mem_size         = 0;

	static PreInitializeController on_init([]() {
		ReflectionInitializeController().require("vkb::DeviceType");

		auto& e = ScriptEngine::instance();

		e.begin_config_group("engine/vulkan.config");
		{
			ScriptNamespaceScopedChanger changer("Engine::Vulkan");
			e.register_property("vkb::DeviceType device_type", &device_type);
			e.register_property("bool enable_validation", &enable_validation);
			e.register_property("bool allow_any_gpu_type", &allow_any_gpu_type);
			e.register_property("bool require_present", &require_present);
			e.register_property("bool require_dedicated_transfer_queue", &require_dedicated_transfer_queue);
			e.register_property("bool require_dedicated_compute_queue", &require_dedicated_compute_queue);
			e.register_property("bool require_separate_transfer_queue", &require_separate_transfer_queue);
			e.register_property("bool require_separate_compute_queue", &require_separate_compute_queue);
		}
		e.end_config_group();
	});
}// namespace Engine::VulkanConfig
