#include <vulkan_api.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_layout.hpp>
#include <vulkan_sampler.hpp>

namespace Trinex
{
	VulkanBindingLayout::VulkanBindingLayout(RHIBinding* bindings, usize count)
	{
		StackByteAllocator::Mark mark;

		vk::Device* device                          = VulkanAPI::device();
		vk::DescriptorSetLayoutBinding* descriptors = StackAllocator<vk::DescriptorSetLayoutBinding>::allocate(count);

		for (usize i = 0; i < count; ++i)
		{
			StackByteAllocator::Mark mark;
			vk::Sampler* samplers = nullptr;

			if (bindings[i].samplers)
			{
				samplers = StackAllocator<vk::Sampler>::allocate(bindings[i].count);

				for (usize idx = 0; idx < bindings[i].count; ++i)
				{
					samplers[idx] = static_cast<VulkanSampler*>(bindings[i].samplers[idx])->sampler();
				}
			}

			descriptors[i].binding            = bindings[i].binding;
			descriptors[i].descriptorCount    = bindings[i].count;
			descriptors[i].pImmutableSamplers = samplers;
			descriptors[i].descriptorType     = VulkanEnums::descriptor_type_of(bindings[i].type);
			descriptors[i].stageFlags         = VulkanEnums::shader_stage_flags_of(bindings[i].stages);
		}

		vk::DescriptorSetLayoutCreateInfo info({}, count, descriptors);
		m_layout = vk::verify(device->createDescriptorSetLayout(info));
	}

	VulkanBindingLayout::~VulkanBindingLayout()
	{
		VulkanAPI::device()->destroyDescriptorSetLayout(m_layout);
	}

	RHI* VulkanBindingLayout::device() const
	{
		return VulkanAPI::instance();
	}

	RHIBindingLayout* VulkanAPI::create_binding_layout(RHIBinding* bindings, usize count)
	{
		return trx_new VulkanBindingLayout(bindings, count);
	}
} // namespace Trinex
