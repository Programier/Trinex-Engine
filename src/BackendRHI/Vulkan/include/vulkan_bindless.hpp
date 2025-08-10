#pragma once
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanBuffer;

	class VulkanDescriptorHeap
	{
	public:
		enum HeapType
		{
			Sampler              = 0,
			CombinedImageSampler = 1,
			SampledImage         = 2,
			StorageImage         = 3,
			UniformTexelBuffer   = 4,
			StorageTexelBuffer   = 5,
			UniformBuffer        = 6,
			StorageBuffer        = 7,
			HeapsCount           = 8
		};

		static constexpr vk::DescriptorType static_descriptor_type(HeapType type)
		{
			switch (type)
			{
				case Sampler: return vk::DescriptorType::eSampler;
				case CombinedImageSampler: return vk::DescriptorType::eCombinedImageSampler;
				case SampledImage: return vk::DescriptorType::eSampledImage;
				case StorageImage: return vk::DescriptorType::eStorageImage;
				case UniformTexelBuffer: return vk::DescriptorType::eUniformTexelBuffer;
				case StorageTexelBuffer: return vk::DescriptorType::eStorageTexelBuffer;
				case UniformBuffer: return vk::DescriptorType::eUniformBuffer;
				case StorageBuffer: return vk::DescriptorType::eStorageBuffer;
				default: return vk::DescriptorType::eSampler;
			}
		}

	private:
		vk::DescriptorSetLayoutBinding m_bindings[HeapsCount];
		Vector<RHIDescriptor> m_free[HeapsCount];
		RHIDescriptor m_last[HeapsCount] = {0};

		vk::DescriptorSetLayout m_descriptor_set_layout;
		vk::DescriptorSet m_descriptor_set;
		vk::DescriptorPool m_descriptor_pool;

	private:
		VulkanDescriptorHeap& initialize(HeapType heap, vk::DescriptorType type, size_t count);
		VulkanDescriptorHeap& create();
		RHIDescriptor allocate(HeapType type);

	public:
		VulkanDescriptorHeap();
		~VulkanDescriptorHeap();
		RHIDescriptor allocate(vk::Sampler sampler);
		RHIDescriptor allocate(vk::ImageView view, HeapType heap);
		RHIDescriptor allocate(VulkanBuffer* buffer, uint64_t offset, uint64_t size, HeapType heap);
		VulkanDescriptorHeap& release(RHIDescriptor descriptor, HeapType heap);

		inline vk::DescriptorSetLayout descriptor_set_layout() const { return m_descriptor_set_layout; }
		inline vk::DescriptorSet descriptor_set() const { return m_descriptor_set; }
	};
}// namespace Engine
