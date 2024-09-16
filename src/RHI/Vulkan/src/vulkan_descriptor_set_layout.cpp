#include <vulkan_api.hpp>
#include <vulkan_descript_set_layout.hpp>

namespace Engine
{
	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		DESTROY_CALL(destroyDescriptorSetLayout, layout);
	}
}// namespace Engine
