#include <ScriptEngine/script_binding.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <vulkan_config.hpp>

namespace Trinex::VulkanConfig
{
	GPUType device_type     = GPUType::Discrete;
	bool enable_validation  = static_cast<bool>(TRINEX_DEBUG_BUILD);
	bool allow_any_gpu_type = false;

	trinex_on_bootstrap()
	{
		ScriptBinding::Enum reg("Trinex::VulkanGPU");
		reg.value("other", GPUType::Other);
		reg.value("integrated", GPUType::Integrated);
		reg.value("discrete", GPUType::Discrete);
		reg.value("virtual_gpu", GPUType::Virtual);
		reg.value("cpu", GPUType::CPU);

		ScriptEngine::begin_config_group("engine/vulkan.config");
		{
			ScriptNamespaceScopedChanger changer("Trinex::Vulkan");
			ScriptEngine::register_property("Trinex::VulkanGPU device_type", &device_type);
			ScriptEngine::register_property("bool enable_validation", &enable_validation);
			ScriptEngine::register_property("bool allow_any_gpu_type", &allow_any_gpu_type);
		}
		ScriptEngine::end_config_group();
	}
} // namespace Trinex::VulkanConfig
