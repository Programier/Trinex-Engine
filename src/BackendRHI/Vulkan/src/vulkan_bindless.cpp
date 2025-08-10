#include <Core/exception.hpp>
#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descriptor.hpp>

namespace Engine
{
	static inline const char* heap_name(VulkanDescriptorHeap::HeapType type)
	{
		switch (type)
		{
			case VulkanDescriptorHeap::Sampler: return "Sampler";
			case VulkanDescriptorHeap::CombinedImageSampler: return "CombinedImageSampler";
			case VulkanDescriptorHeap::SampledImage: return "SampledImage";
			case VulkanDescriptorHeap::StorageImage: return "StorageImage";
			case VulkanDescriptorHeap::UniformTexelBuffer: return "UniformTexelBuffer";
			case VulkanDescriptorHeap::StorageTexelBuffer: return "StorageTexelBuffer";
			case VulkanDescriptorHeap::UniformBuffer: return "UniformBuffer";
			case VulkanDescriptorHeap::StorageBuffer: return "StorageBuffer";
			default: return "Undefined";
		}
	}

	VulkanDescriptorHeap::VulkanDescriptorHeap()
	{
		initialize(Sampler, vk::DescriptorType::eSampler, 2048);
		initialize(CombinedImageSampler, vk::DescriptorType::eCombinedImageSampler, 256 * 1024);
		initialize(SampledImage, vk::DescriptorType::eSampledImage, 256 * 1024);
		initialize(StorageImage, vk::DescriptorType::eStorageImage, 64 * 1024);
		initialize(UniformTexelBuffer, vk::DescriptorType::eUniformTexelBuffer, 64 * 1024);
		initialize(StorageTexelBuffer, vk::DescriptorType::eStorageTexelBuffer, 64 * 1024);
		initialize(UniformBuffer, vk::DescriptorType::eUniformBuffer, 768 * 1024);
		initialize(StorageBuffer, vk::DescriptorType::eStorageBuffer, 768 * 1024);

		create();
	}

	VulkanDescriptorHeap::~VulkanDescriptorHeap()
	{
		API->m_device.destroyDescriptorPool(m_descriptor_pool);
		API->m_device.destroyDescriptorSetLayout(m_descriptor_set_layout);
	}

	VulkanDescriptorHeap& VulkanDescriptorHeap::initialize(HeapType heap, vk::DescriptorType type, size_t count)
	{
		new (&m_bindings[heap]) vk::DescriptorSetLayoutBinding(heap, type, count, vk::ShaderStageFlagBits::eAll);
		return *this;
	}

	VulkanDescriptorHeap& VulkanDescriptorHeap::create()
	{
		// Create descriptor set layout
		{
			const vk::DescriptorSetLayoutCreateFlags layout_flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

			const vk::DescriptorBindingFlagsEXT binding_flags = vk::DescriptorBindingFlagBits::eUpdateAfterBind |
			                                                    vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending |
			                                                    vk::DescriptorBindingFlagBits::ePartiallyBound;

			vk::DescriptorBindingFlagsEXT binding_flags_array[HeapsCount];
			for (uint_t i = 0; i < HeapsCount; ++i) binding_flags_array[i] = binding_flags;

			vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT binding_info(HeapsCount, binding_flags_array);
			vk::DescriptorSetLayoutCreateInfo layout_info(layout_flags, HeapsCount, m_bindings, &binding_info);
			m_descriptor_set_layout = API->m_device.createDescriptorSetLayout(layout_info);
		}

		// Create descriptor pool
		{
			vk::DescriptorPoolCreateFlags flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
			vk::DescriptorPoolSize pool_size[HeapsCount];

			for (uint_t i = 0; i < HeapsCount; ++i)
			{
				HeapType type = static_cast<HeapType>(i);
				pool_size[type].setType(m_bindings[type].descriptorType);
				pool_size[type].setDescriptorCount(m_bindings[type].descriptorCount);

				m_free[i].clear();
				m_last[i] = 0;
			}

			vk::DescriptorPoolCreateInfo info(flags, 1, HeapsCount, pool_size);
			m_descriptor_pool = API->m_device.createDescriptorPool(info);
		}

		// Create descriptor set
		{
			vk::DescriptorSetAllocateInfo info(m_descriptor_pool, 1, &m_descriptor_set_layout);
			m_descriptor_set = API->m_device.allocateDescriptorSets(info)[0];
		}
		return *this;
	}

	RHIDescriptor VulkanDescriptorHeap::allocate(HeapType type)
	{
		auto& free = m_free[type];

		if (!free.empty())
		{
			RHIDescriptor descriptor = free.back();
			free.pop_back();
			return descriptor;
		}

		if (m_last[type] >= m_bindings[type].descriptorCount)
		{
			String msg = Strings::format("Cannot allocate descriptor in heap '%s'", heap_name(type));
			throw EngineException(msg);
		}

		return m_last[type]++;
	}

	RHIDescriptor VulkanDescriptorHeap::allocate(vk::Sampler sampler)
	{
		RHIDescriptor descriptor = allocate(Sampler);

		vk::DescriptorImageInfo image_info(sampler, {}, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet write(m_descriptor_set, Sampler, descriptor, vk::DescriptorType::eSampler, image_info);
		API->m_device.updateDescriptorSets(write, {});
		return descriptor;
	}

	RHIDescriptor VulkanDescriptorHeap::allocate(vk::ImageView view, HeapType heap)
	{
		RHIDescriptor descriptor = allocate(heap);

		if (heap == SampledImage)
		{
			vk::DescriptorImageInfo image_info({}, view, vk::ImageLayout::eShaderReadOnlyOptimal);
			vk::WriteDescriptorSet write(m_descriptor_set, heap, descriptor, vk::DescriptorType::eSampledImage, image_info);
			API->m_device.updateDescriptorSets(write, {});
		}
		else if (heap == StorageImage)
		{
			vk::DescriptorImageInfo image_info({}, view, vk::ImageLayout::eGeneral);
			vk::WriteDescriptorSet write(m_descriptor_set, heap, descriptor, vk::DescriptorType::eStorageImage, image_info);
			API->m_device.updateDescriptorSets(write, {});
		}
		else
		{
			throw EngineException("Unsupported heap type!");
		}

		return descriptor;
	}

	RHIDescriptor VulkanDescriptorHeap::allocate(VulkanBuffer* buffer, uint64_t offset, uint64_t size, HeapType heap)
	{
		RHIDescriptor descriptor           = allocate(heap);
		vk::DescriptorType descriptor_type = static_descriptor_type(heap);

		vk::DescriptorBufferInfo buffer_info(buffer->buffer(), offset, size);
		vk::WriteDescriptorSet write(m_descriptor_set, heap, descriptor, descriptor_type, {}, buffer_info, {});
		API->m_device.updateDescriptorSets(write, {});

		return descriptor;
	}

	VulkanDescriptorHeap& VulkanDescriptorHeap::release(RHIDescriptor descriptor, HeapType heap)
	{
		m_free[heap].push_back(descriptor);
		return *this;
	}
}// namespace Engine
