#include <vulkan_api.hpp>
#include <vulkan_descript_set_layout.hpp>

namespace Engine
{
	void VulkanDescriptorSetLayout::destroy()
	{
		DESTROY_CALL(destroyDescriptorSetLayout, layout);
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		destroy();
	}
}// namespace Engine
