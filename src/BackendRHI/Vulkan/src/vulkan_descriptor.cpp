#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	VulkanPipelineLayout::VulkanPipelineLayout(size_t hash, vk::ShaderStageFlags stages, Descriptor* descriptors, size_t count)
	    : m_descriptors(nullptr), m_hash(hash), m_descriptors_count(count)
	{
		m_descriptors = Allocator<Descriptor>::allocate(count);
		std::memcpy(m_descriptors, descriptors, sizeof(Descriptor) * count);

		StackByteAllocator::Mark mark;

		vk::PipelineLayoutCreateInfo pipeline_info;

		if (count > 0)
		{
			vk::DescriptorSetLayoutBinding* bindings = StackAllocator<vk::DescriptorSetLayoutBinding>::allocate(count);

			for (size_t i = 0; i < count; ++i)
			{
				auto dst = bindings + i;
				auto src = descriptors + i;
				new (dst) vk::DescriptorSetLayoutBinding(src->binding, src->type, 1, stages);

				switch (src->type)
				{
					case vk::DescriptorType::eSampledImage: ++m_textures; break;
					case vk::DescriptorType::eSampler: ++m_samplers; break;
					case vk::DescriptorType::eCombinedImageSampler: ++m_combined_image_sampler; break;
					case vk::DescriptorType::eStorageImage: ++m_storage_images; break;

					case vk::DescriptorType::eUniformBuffer: ++m_uniform_buffers; break;
					case vk::DescriptorType::eStorageBuffer: ++m_storage_buffers; break;
					case vk::DescriptorType::eUniformTexelBuffer: ++m_uniform_texel_buffers; break;
					case vk::DescriptorType::eStorageTexelBuffer: ++m_storage_texel_buffers; break;
					case vk::DescriptorType::eAccelerationStructureKHR: ++m_acceleration_structures; break;
					default: throw EngineException("Unimplemented descriptor type!");
				}
			}

			vk::DescriptorSetLayoutCreateInfo info({}, count, bindings);
			m_set_layout                       = API->m_device.createDescriptorSetLayout(info);
			vk::DescriptorSetLayout layouts[2] = {m_set_layout, API->descriptor_heap()->descriptor_set_layout()};
			pipeline_info.setSetLayouts(layouts);
		}

		m_layout = API->m_device.createPipelineLayout(pipeline_info);
	}

	VulkanPipelineLayout::~VulkanPipelineLayout()
	{
		trx_delete m_descriptors;
		API->m_device.destroyPipelineLayout(m_layout);

		if (m_set_layout)
		{
			API->m_device.destroyDescriptorSetLayout(m_set_layout);
		}
	}

	void VulkanPipelineLayout::destroy()
	{
		API->destroy_pipeline_layout(this);
	}

	struct VulkanDescriptorSetAllocator::VulkanDescriptorPool {
		static constexpr inline uint32_t s_descriptor_sets_per_pool = 1024;

	private:
		vk::DescriptorPool m_pool;

		uint32_t m_descriptors;

		union
		{
			struct {
				uint32_t m_textures;
				uint32_t m_samplers;
				uint32_t m_combined_image_sampler;
				uint32_t m_storage_images;
				uint32_t m_uniform_buffers;
				uint32_t m_storage_buffers;
				uint32_t m_uniform_texel_buffers;
				uint32_t m_storage_texel_buffers;
				uint32_t m_acceleration_structures;
			};

			uint32_t m_pool_sizes[9];
		};

		VulkanFenceRef m_fence;

		void reset_counters()
		{
			m_descriptors             = s_descriptor_sets_per_pool;
			m_textures                = s_descriptor_sets_per_pool * 4;
			m_samplers                = s_descriptor_sets_per_pool * 2;
			m_combined_image_sampler  = s_descriptor_sets_per_pool * 8;
			m_storage_images          = s_descriptor_sets_per_pool * 2;
			m_uniform_buffers         = s_descriptor_sets_per_pool * 6;
			m_storage_buffers         = s_descriptor_sets_per_pool * 4;
			m_uniform_texel_buffers   = s_descriptor_sets_per_pool * 1;
			m_storage_texel_buffers   = s_descriptor_sets_per_pool * 1;
			m_acceleration_structures = s_descriptor_sets_per_pool * 1;
		}

	public:
		VulkanDescriptorPool* next = nullptr;

		VulkanDescriptorPool()
		{
			reset_counters();

			vk::DescriptorPoolSize sizes[] = {
			        {vk::DescriptorType::eSampledImage, m_textures},
			        {vk::DescriptorType::eSampler, m_samplers},
			        {vk::DescriptorType::eCombinedImageSampler, m_combined_image_sampler},
			        {vk::DescriptorType::eStorageImage, m_storage_images},
			        {vk::DescriptorType::eUniformBuffer, m_uniform_buffers},
			        {vk::DescriptorType::eStorageBuffer, m_storage_buffers},
			        {vk::DescriptorType::eUniformTexelBuffer, m_uniform_texel_buffers},
			        {vk::DescriptorType::eStorageTexelBuffer, m_storage_texel_buffers},
			        {vk::DescriptorType::eAccelerationStructureKHR, m_acceleration_structures},
			};

			size_t count = API->is_raytracing_supported() ? 9 : 8;

			vk::DescriptorPoolCreateInfo info({}, s_descriptor_sets_per_pool, count, sizes);
			m_pool = API->m_device.createDescriptorPool(info);
		}

		~VulkanDescriptorPool() { API->m_device.destroyDescriptorPool(m_pool); }

		VulkanDescriptorPool& reset()
		{
			reset_counters();
			trinex_profile_cpu_n("VulkanDescriptorPool reset");
			API->m_device.resetDescriptorPool(m_pool);
			return *this;
		}

		VulkanDescriptorPool& submit()
		{
			m_fence.signal(API->current_command_buffer());
			return *this;
		}

		bool is_free() { return m_fence.is_signaled() || !m_fence.is_waiting(); }

		vk::DescriptorSet allocate(VulkanPipelineLayout* layout)
		{
			if (m_descriptors == 0)
				return {};

			{
				trinex_profile_cpu_n("Check available");
				const uint32_t pool_sizes[] = {
				        layout->textures_count(),
				        layout->samplers_count(),
				        layout->combined_image_sampler_count(),
				        layout->storage_images_count(),
				        layout->uniform_buffers_count(),
				        layout->storage_buffers_count(),
				        layout->uniform_texel_buffers_count(),
				        layout->storage_texel_buffers_count(),
				        layout->acceleartion_structures_count(),
				};

				static constexpr uint32_t count = sizeof(m_pool_sizes) / sizeof(m_pool_sizes[0]);

				for (uint32_t i = 0; i < count; ++i)
				{
					if (pool_sizes[i] > m_pool_sizes[i])
						return {};

					m_pool_sizes[i] -= pool_sizes[i];
				}
			}


			trinex_profile_cpu_n("Allocate");
			vk::DescriptorSetLayout descriptor_set_layout = layout->descriptor_set_layout();
			vk::DescriptorSetAllocateInfo info(m_pool, descriptor_set_layout);
			vk::DescriptorSet set;
			vk::Result result = API->m_device.allocateDescriptorSets(&info, &set);

			if (result == vk::Result::eSuccess)
				return set;
			return {};
		}
	};

	VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator()
	{
		m_current  = trx_new VulkanDescriptorPool();
		m_pool     = trx_new VulkanDescriptorPool();
		m_push_ptr = &m_pool->next;
	}

	VulkanDescriptorSetAllocator::~VulkanDescriptorSetAllocator()
	{
		trx_delete m_current;

		while (m_pool)
		{
			m_current = m_pool;
			m_pool    = m_pool->next;

			trx_delete m_current;
		}
	}

	vk::DescriptorSet VulkanDescriptorSetAllocator::allocate(VulkanPipelineLayout* layout)
	{
		trinex_profile_cpu_n("VulkanDescriptorSetAllocator::allocate");
		vk::DescriptorSet set;
		while (!(set = m_current->allocate(layout))) submit();
		return set;
	}

	VulkanDescriptorSetAllocator& VulkanDescriptorSetAllocator::submit()
	{
		trinex_profile_cpu_n("VulkanDescriptorSetAllocator::submit");

		(*m_push_ptr) = m_current;
		m_push_ptr    = &m_current->next;
		m_current->submit();

		if (m_pool->is_free())
		{
			m_current       = m_pool;
			m_pool          = m_pool->next;
			m_current->next = nullptr;
			m_current->reset();
		}
		else
		{
			m_current = trx_new VulkanDescriptorPool();
		}
		return *this;
	}

	VulkanPipelineLayout* VulkanAPI::create_pipeline_layout(const RHIShaderParameterInfo* parameters, size_t count,
	                                                        vk::ShaderStageFlags stages)
	{
		using Descriptor = VulkanPipelineLayout::Descriptor;

		StackByteAllocator::Mark mark;
		Descriptor* descriptors = StackAllocator<Descriptor>::allocate(count);

		size_t descriptors_count = 0;

		for (size_t i = 0; i < count; ++i)
		{
			auto type    = VulkanEnums::descriptor_type_of(parameters[i].type);
			auto binding = parameters[i].binding;

			for (size_t j = 0; j < descriptors_count; ++j)
			{
				if (descriptors[j].type == type && descriptors[j].binding == binding)
				{
					goto next_parameter;
				}
			}

			descriptors[descriptors_count].binding = binding;
			descriptors[descriptors_count].type    = type;
			++descriptors_count;

		next_parameter:;
		}

		std::sort(descriptors, descriptors + descriptors_count);

		uint64_t hash = static_cast<uint64_t>(static_cast<VkShaderStageFlags>(stages));
		hash          = memory_hash(descriptors, descriptors_count * sizeof(Descriptor), hash);

		auto search_result = m_pipeline_layouts.equal_range(hash);

		while (search_result.first != search_result.second)
		{
			VulkanPipelineLayout* layout = search_result.first->second;
			if (layout->equals(descriptors, descriptors_count))
			{
				layout->add_reference();
				return layout;
			}
			++search_result.first;
		}

		// Layout is not found, create new one
		VulkanPipelineLayout* layout = trx_new VulkanPipelineLayout(hash, stages, descriptors, descriptors_count);
		m_pipeline_layouts.insert({hash, layout});
		return layout;
	}

	VulkanAPI& VulkanAPI::destroy_pipeline_layout(VulkanPipelineLayout* layout)
	{
		auto search_result = m_pipeline_layouts.equal_range(layout->hash());

		while (search_result.first != search_result.second)
		{
			VulkanPipelineLayout* target = search_result.first->second;

			if (target->equals(layout->descriptors(), layout->descriptors_count()))
			{
				m_pipeline_layouts.erase(search_result.first);
				trx_delete layout;
				return *this;
			}
			++search_result.first;
		}
		return *this;
	}
}// namespace Engine
