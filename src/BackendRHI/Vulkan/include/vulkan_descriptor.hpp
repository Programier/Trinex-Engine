#pragma once
#include <Core/etl/critical_section.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_thread_local.hpp>

namespace Trinex
{
	class VulkanContext;

	class VulkanPipelineLayout : public RHIObject
	{
	public:
		struct Descriptor {
			vk::DescriptorType type;
			u32 binding;

			inline bool operator==(const Descriptor& other) const { return type == other.type && binding == other.binding; }
			inline bool operator!=(const Descriptor& other) const { return type != other.type || binding != other.binding; }
			inline bool operator<(const Descriptor& other) const
			{
				const bool is_this_uniform  = (type == vk::DescriptorType::eUniformBufferDynamic);
				const bool is_other_uniform = (other.type == vk::DescriptorType::eUniformBufferDynamic);

				if (is_this_uniform != is_other_uniform)
					return is_this_uniform;

				if (type != other.type)
					return static_cast<u32>(type) < static_cast<u32>(other.type);

				return binding < other.binding;
			}
		};

	private:
		vk::DescriptorSetLayout m_set_layout;
		vk::PipelineLayout m_layout;
		Descriptor* m_descriptors     = nullptr;
		u64 m_hash               = 0;
		u16 m_descriptors_count  = 0;
		u8 m_textures               = 0;
		u8 m_samplers               = 0;
		u8 m_combined_image_sampler = 0;
		u8 m_storage_images         = 0;

		u8 m_uniform_buffers         = 0;
		u8 m_storage_buffers         = 0;
		u8 m_uniform_texel_buffers   = 0;
		u8 m_storage_texel_buffers   = 0;
		u8 m_acceleration_structures = 0;

	public:
		VulkanPipelineLayout(u64 hash, vk::ShaderStageFlags stages, Descriptor* descriptors, usize count);
		~VulkanPipelineLayout();
		void destroy() override;

		inline vk::DescriptorSetLayout descriptor_set_layout() const { return m_set_layout; }
		inline bool has_descriptor_set() const { return m_descriptors_count != 0; }
		inline vk::PipelineLayout layout() const { return m_layout; }
		inline usize descriptors_count() const { return m_descriptors_count; }
		inline Descriptor descriptor(usize index) const { return m_descriptors[index]; }
		inline u64 hash() const { return m_hash; }
		const Descriptor* descriptors() const { return m_descriptors; }

		inline u8 textures_count() const { return m_textures; };
		inline u8 samplers_count() const { return m_samplers; };
		inline u8 combined_image_sampler_count() const { return m_combined_image_sampler; };
		inline u8 storage_images_count() const { return m_storage_images; };

		inline u8 uniform_buffers_count() const { return m_uniform_buffers; }
		inline u8 storage_buffers_count() const { return m_storage_buffers; }
		inline u8 uniform_texel_buffers_count() const { return m_uniform_texel_buffers; }
		inline u8 storage_texel_buffers_count() const { return m_storage_texel_buffers; }
		inline u8 acceleartion_structures_count() const { return m_acceleration_structures; }

		inline bool equals(const Descriptor* descriptors, usize count)
		{
			if (m_descriptors_count != count)
				return false;

			for (usize i = 0; i < count; ++i)
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
		MultiMap<u64, class VulkanPipelineLayout*> m_pipeline_layouts;

	public:
		VulkanPipelineLayout* allocate(const RHIShaderParameterInfo* parameters, usize count, vk::ShaderStageFlags stages);
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

		Map<u64, vk::DescriptorSet> m_table;

		vk::DescriptorSet allocate(VulkanPipelineLayout* layout);

	public:
		VulkanDescriptorSetAllocator();
		~VulkanDescriptorSetAllocator();
		vk::DescriptorSet allocate(VulkanPipelineLayout* layout, VulkanContext* context);

		static VulkanDescriptorSetAllocator* instance();
	};
}// namespace Trinex
