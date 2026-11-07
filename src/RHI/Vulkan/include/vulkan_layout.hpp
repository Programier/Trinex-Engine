#pragma once
#include <RHI/handles.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanBindingLayout : public VulkanDeferredDestroy<RHIBindingLayout>
	{
	private:
		vk::DescriptorSetLayout m_layout;

	public:
		VulkanBindingLayout(RHIBinding* bindings, usize count);
		~VulkanBindingLayout();
	};
} // namespace Trinex
