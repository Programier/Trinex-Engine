#include <Core/engine_loading_controllers.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <VkBootstrap.h>
#include <vulkan_config.hpp>

namespace vkb
{
	using DeviceType = PreferredDeviceType;
}

namespace Engine::VulkanConfig
{
	vkb::PreferredDeviceType device_type  = vkb::PreferredDeviceType::discrete;
	bool enable_validation                = static_cast<bool>(TRINEX_DEBUG_BUILD);
	bool allow_any_gpu_type               = false;
	bool require_present                  = true;
	bool require_dedicated_transfer_queue = false;
	bool require_dedicated_compute_queue  = false;
	bool require_separate_transfer_queue  = false;
	bool require_separate_compute_queue   = false;
	unsigned int required_mem_size        = 0;
	unsigned int desired_mem_size         = 0;

	static PreInitializeController on_init([]() {
		ScriptEnumRegistrar reg("Engine::VulkanGPU");
		reg.set("other", vkb::DeviceType::other);
		reg.set("integrated", vkb::DeviceType::integrated);
		reg.set("discrete", vkb::DeviceType::discrete);
		reg.set("virtual_gpu", vkb::DeviceType::virtual_gpu);
		reg.set("cpu", vkb::DeviceType::cpu);

		auto& e = ScriptEngine::instance();

		e.begin_config_group("engine/vulkan.config");
		{
			ScriptNamespaceScopedChanger changer("Engine::Vulkan");
			e.register_property("Engine::VulkanGPU device_type", &device_type);
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
