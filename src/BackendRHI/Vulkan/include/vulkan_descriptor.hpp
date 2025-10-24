#pragma once
#include <Core/etl/critical_section.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_thread_local.hpp>

namespace Engine
{
	class VulkanStateManager;

	class VulkanPipelineLayout : public RHIObject
	{
	public:
		struct Descriptor {
			vk::DescriptorType type;
			uint32_t binding;

			inline bool operator==(const Descriptor& other) const { return type == other.type && binding == other.binding; }
			inline bool operator!=(const Descriptor& other) const { return type != other.type || binding != other.binding; }
			inline bool operator<(const Descriptor& other) const
			{
				const bool is_this_uniform  = (type == vk::DescriptorType::eUniformBufferDynamic);
				const bool is_other_uniform = (other.type == vk::DescriptorType::eUniformBufferDynamic);

				if (is_this_uniform != is_other_uniform)
					return is_this_uniform;

				if (type != other.type)
					return static_cast<uint32_t>(type) < static_cast<uint32_t>(other.type);

				return binding < other.binding;
			}
		};

	private:
		vk::DescriptorSetLayout m_set_layout;
		vk::PipelineLayout m_layout;
		Descriptor* m_descriptors     = nullptr;
		uint64_t m_hash               = 0;
		uint16_t m_descriptors_count  = 0;
		byte m_textures               = 0;
		byte m_samplers               = 0;
		byte m_combined_image_sampler = 0;
		byte m_storage_images         = 0;

		byte m_uniform_buffers         = 0;
		byte m_storage_buffers         = 0;
		byte m_uniform_texel_buffers   = 0;
		byte m_storage_texel_buffers   = 0;
		byte m_acceleration_structures = 0;

	public:
		VulkanPipelineLayout(uint64_t hash, vk::ShaderStageFlags stages, Descriptor* descriptors, size_t count);
		~VulkanPipelineLayout();
		void destroy() override;

		inline vk::DescriptorSetLayout descriptor_set_layout() const { return m_set_layout; }
		inline bool has_descriptor_set() const { return m_descriptors_count != 0; }
		inline vk::PipelineLayout layout() const { return m_layout; }
		inline size_t descriptors_count() const { return m_descriptors_count; }
		inline Descriptor descriptor(size_t index) const { return m_descriptors[index]; }
		inline uint64_t hash() const { return m_hash; }
		const Descriptor* descriptors() const { return m_descriptors; }

		inline byte textures_count() const { return m_textures; };
		inline byte samplers_count() const { return m_samplers; };
		inline byte combined_image_sampler_count() const { return m_combined_image_sampler; };
		inline byte storage_images_count() const { return m_storage_images; };

		inline byte uniform_buffers_count() const { return m_uniform_buffers; }
		inline byte storage_buffers_count() const { return m_storage_buffers; }
		inline byte uniform_texel_buffers_count() const { return m_uniform_texel_buffers; }
		inline byte storage_texel_buffers_count() const { return m_storage_texel_buffers; }
		inline byte acceleartion_structures_count() const { return m_acceleration_structures; }

		inline bool equals(const Descriptor* descriptors, size_t count)
		{
			if (m_descriptors_count != count)
				return false;

			for (size_t i = 0; i < count; ++i)
			{
				if (m_descriptors[i] != descriptors[i])
					return false;
			}
			return true;
		}

		friend class VulkanPipelineLayoutManager;
	};

	class VulkanPipelineLayoutManager
	{
	private:
		CriticalSection m_section;
		MultiMap<uint64_t, class VulkanPipelineLayout*> m_pipeline_layouts;

	public:
		VulkanPipelineLayout* allocate(const RHIShaderParameterInfo* parameters, size_t count, vk::ShaderStageFlags stages);
		VulkanPipelineLayoutManager& desctroy(VulkanPipelineLayout* layout);
	};

	class VulkanDescriptorSetAllocator : public VulkanThreadLocal
	{
	public:
		struct VulkanDescriptorPool;
		struct Binding;

	private:
		VulkanDescriptorPool* m_pool      = nullptr;
		VulkanDescriptorPool** m_push_ptr = nullptr;

		Map<uint64_t, vk::DescriptorSet> m_table;

		vk::DescriptorSet allocate(VulkanPipelineLayout* layout);

	public:
		VulkanDescriptorSetAllocator();
		~VulkanDescriptorSetAllocator();
		vk::DescriptorSet allocate(VulkanPipelineLayout* layout, VulkanStateManager* state);

		static VulkanDescriptorSetAllocator* instance();
	};
}// namespace Engine
