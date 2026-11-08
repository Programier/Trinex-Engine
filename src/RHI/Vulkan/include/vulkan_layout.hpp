#pragma once
#include <RHI/handles.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	class VulkanBindingLayout final : public RHIDirectDestroy<RHIBindingLayout>
	{
	private:
		RHIGarbageGroup* m_garbage_group = nullptr;
		vk::DescriptorSetLayout m_layout;

	public:
		VulkanBindingLayout(RHIBinding* bindings, usize count);
		~VulkanBindingLayout();

		RHI* device() const override;
		inline RHIGarbageGroup* binding_set_garbage_group() const { return m_garbage_group; }
	};
} // namespace Trinex
