#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <VkBootstrap.h>
#include <vulkan_config.hpp>

namespace vkb
{
	using DeviceType = PreferredDeviceType;
}

namespace Trinex::VulkanConfig
{
	vkb::PreferredDeviceType device_type = vkb::PreferredDeviceType::discrete;
	bool enable_validation               = static_cast<bool>(TRINEX_DEBUG_BUILD);
	bool allow_any_gpu_type              = false;

	trinex_on_pre_init()
	{
		ScriptBinding::Enum reg("Trinex::VulkanGPU");
		reg.value("other", vkb::DeviceType::other);
		reg.value("integrated", vkb::DeviceType::integrated);
		reg.value("discrete", vkb::DeviceType::discrete);
		reg.value("virtual_gpu", vkb::DeviceType::virtual_gpu);
		reg.value("cpu", vkb::DeviceType::cpu);

		auto& e = ScriptEngine::instance();

		e.begin_config_group("engine/vulkan.config");
		{
			ScriptNamespaceScopedChanger changer("Trinex::Vulkan");
			e.register_property("Trinex::VulkanGPU device_type", &device_type);
			e.register_property("bool enable_validation", &enable_validation);
			e.register_property("bool allow_any_gpu_type", &allow_any_gpu_type);
		}
		e.end_config_group();
	}
}// namespace Trinex::VulkanConfig
