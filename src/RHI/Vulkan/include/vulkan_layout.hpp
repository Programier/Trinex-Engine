#pragma once
#include <RHI/handles.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanBindingLayout : public RHIBindingLayout
	{
	private:
		vk::DescriptorSetLayout m_layout;

	public:
		VulkanBindingLayout(RHIBinding* bindings, usize count);
		~VulkanBindingLayout();

		RHI* device() const override;
	};
} // namespace Trinex
