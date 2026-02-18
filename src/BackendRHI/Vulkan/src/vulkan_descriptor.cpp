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
#include <vulkan_state.hpp>
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

					case vk::DescriptorType::eUniformBufferDynamic: ++m_uniform_buffers; break;
					case vk::DescriptorType::eStorageBuffer: ++m_storage_buffers; break;
					case vk::DescriptorType::eUniformTexelBuffer: ++m_uniform_texel_buffers; break;
					case vk::DescriptorType::eStorageTexelBuffer: ++m_storage_texel_buffers; break;
					case vk::DescriptorType::eAccelerationStructureKHR: ++m_acceleration_structures; break;
					default: trinex_unreachable_msg("Unimplemented descriptor type!");
				}
			}

			vk::DescriptorSetLayoutCreateInfo info({}, count, bindings);
			m_set_layout = API->m_device.createDescriptorSetLayout(info);

			auto layouts = StackAllocator<vk::DescriptorSetLayout>::allocate(2);
			layouts[0]   = m_set_layout;
			layouts[1]   = API->descriptor_heap()->descriptor_set_layout();
			pipeline_info.setPSetLayouts(layouts).setSetLayoutCount(2);
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
		API->pipeline_layout_manager()->desctroy(this);
	}

	VulkanPipelineLayout* VulkanPipelineLayoutManager::allocate(const RHIShaderParameterInfo* parameters, size_t count,
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

		ScopeLock lock(m_section);
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

	VulkanPipelineLayoutManager& VulkanPipelineLayoutManager::desctroy(VulkanPipelineLayout* layout)
	{
		ScopeLock lock(m_section);
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
			        {vk::DescriptorType::eUniformBufferDynamic, m_uniform_buffers},
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

	struct VulkanDescriptorSetAllocator::Binding {
		struct Descriptor {
			using enum vk::ImageLayout;

			uint64_t data[2];

			inline void zeros() { data[0] = data[1] = 0; }

			template<typename T>
			inline Descriptor& operator=(const T& object)
			{
				static_assert(alignof(T) == alignof(Descriptor));
				static_assert(sizeof(T) <= sizeof(Descriptor));

				const uint64_t* src = reinterpret_cast<const uint64_t*>(&object);

				data[0] = src[0];

				if constexpr (sizeof(T) > 8)
					data[1] = src[1];
				else
					data[1] = 0;

				return *this;
			}

			inline Descriptor& operator=(const VulkanStateManager::UniformBuffer& buffer)
			{
				VulkanStateManager::UniformBuffer& dst = as<VulkanStateManager::UniformBuffer>();
				dst.buffer                             = buffer.buffer;
				dst.size                               = buffer.size;
				dst.offset                             = buffer.offset;
				return *this;
			}

			template<typename T>
			inline const T& as() const
			{
				static_assert(alignof(T) == alignof(Descriptor));
				static_assert(sizeof(T) <= sizeof(Descriptor));
				return *reinterpret_cast<const T*>(data);
			}

			template<typename T>
			inline T& as()
			{
				static_assert(alignof(T) == alignof(Descriptor));
				static_assert(sizeof(T) <= sizeof(Descriptor));
				return *reinterpret_cast<T*>(data);
			}

			inline void write_sampled_image(vk::WriteDescriptorSet* write)
			{
				const VulkanTextureSRV* srv = as<VulkanTextureSRV*>();
				write->pImageInfo           = trx_stack_new vk::DescriptorImageInfo({}, srv->view(), eShaderReadOnlyOptimal);
			}

			inline void write_sampler(vk::WriteDescriptorSet* write)
			{
				const vk::Sampler& sampler = as<vk::Sampler>();
				write->pImageInfo          = trx_stack_new vk::DescriptorImageInfo(sampler, {}, eUndefined);
			}

			inline void write_combined_image_sampler(vk::WriteDescriptorSet* write)
			{
				auto& [srv, sampler] = as<VulkanStateManager::CombinedImage>();
				write->pImageInfo    = trx_stack_new vk::DescriptorImageInfo(sampler, srv->view(), eShaderReadOnlyOptimal);
			}

			inline void write_storage_image(vk::WriteDescriptorSet* write)
			{
				VulkanTextureUAV* uav = as<VulkanTextureUAV*>();
				write->pImageInfo     = trx_stack_new vk::DescriptorImageInfo({}, uav->view(), eGeneral);
			}

			inline void write_uniform_buffer(vk::WriteDescriptorSet* write)
			{
				auto& buffer       = as<VulkanStateManager::UniformBuffer>();
				write->pBufferInfo = trx_stack_new vk::DescriptorBufferInfo(buffer.buffer, 0, buffer.size);
			}

			inline void write_buffer(vk::WriteDescriptorSet* write)
			{
				auto& buffer       = as<vk::Buffer>();
				write->pBufferInfo = trx_stack_new vk::DescriptorBufferInfo(buffer, 0, vk::WholeSize);
			}

			inline void write_acceleration(vk::WriteDescriptorSet* write)
			{
				auto& tlas   = as<vk::AccelerationStructureKHR>();
				write->pNext = trx_stack_new vk::WriteDescriptorSetAccelerationStructureKHR(tlas);
			}
		};

		vk::DescriptorType type;
		uint32_t binding;
		Descriptor descriptor;
	};

	VulkanDescriptorSetAllocator::VulkanDescriptorSetAllocator()
	{
		m_pool     = trx_new VulkanDescriptorPool();
		m_push_ptr = &m_pool->next;
	}

	VulkanDescriptorSetAllocator::~VulkanDescriptorSetAllocator()
	{
		VulkanDescriptorPool* current = nullptr;

		while (m_pool)
		{
			current = m_pool;
			m_pool  = m_pool->next;
			trx_delete current;
		}
	}

	vk::DescriptorSet VulkanDescriptorSetAllocator::allocate(VulkanPipelineLayout* layout)
	{
		VulkanDescriptorPool* pool = m_pool;
		vk::DescriptorSet set      = pool->allocate(layout);

		while (!set && pool->next)
		{
			pool = pool->next;
			set  = pool->allocate(layout);
		}

		if (!set)
		{
			pool->next = trx_new VulkanDescriptorPool();
			set        = pool->next->allocate(layout);
		}

		return set;
	}

	vk::DescriptorSet VulkanDescriptorSetAllocator::allocate(VulkanPipelineLayout* layout, VulkanStateManager* state)
	{
		const size_t count     = layout->descriptors_count();
		const auto* descritors = layout->descriptors();
		StackByteAllocator::Mark mark;

		Binding* bindings = StackAllocator<Binding>::allocate(count);

		for (size_t i = 0; i < count; ++i)
		{
			const auto* src = descritors + i;
			Binding* dst    = bindings + i;

			dst->type    = src->type;
			dst->binding = src->binding;

			switch (dst->type)
			{
				case vk::DescriptorType::eSampledImage: dst->descriptor = state->srv_images.resource(dst->binding); break;
				case vk::DescriptorType::eSampler: dst->descriptor = state->samplers.resource(dst->binding); break;
				case vk::DescriptorType::eCombinedImageSampler:
				{
					dst->descriptor = VulkanStateManager::CombinedImage{
					        state->srv_images.resource(dst->binding),
					        state->samplers.resource(dst->binding),
					};
					break;
				}
				case vk::DescriptorType::eStorageImage: dst->descriptor = state->storage_buffers.resource(dst->binding); break;
				case vk::DescriptorType::eUniformBufferDynamic:
				{
					dst->descriptor = state->uniform_buffers.resource(dst->binding);
					break;
				}
				case vk::DescriptorType::eStorageBuffer: dst->descriptor = state->storage_buffers.resource(dst->binding); break;
				case vk::DescriptorType::eUniformTexelBuffer:
					dst->descriptor = state->uniform_texel_buffers.resource(dst->binding);
					break;
				case vk::DescriptorType::eStorageTexelBuffer:
					dst->descriptor = state->storage_texel_buffers.resource(dst->binding);
					break;
				case vk::DescriptorType::eAccelerationStructureKHR:
					dst->descriptor = state->acceleration_structures.resource(dst->binding);
					break;
				default: dst->descriptor.zeros(); break;
			}
		}

		uint64_t hash = memory_hash(bindings, count * sizeof(Binding), reinterpret_cast<uint64_t>(layout));

		vk::DescriptorSet& set = m_table[hash];

		if (set)
			return set;

		set = allocate(layout);

		vk::WriteDescriptorSet* writes = StackAllocator<vk::WriteDescriptorSet>::allocate(count);

		for (size_t i = 0; i < count; ++i)
		{
			Binding* src                = bindings + i;
			vk::WriteDescriptorSet* dst = writes + i;
			new (dst) vk::WriteDescriptorSet(set, src->binding, 0, 1, src->type);

			switch (src->type)
			{
				case vk::DescriptorType::eSampledImage: src->descriptor.write_sampled_image(dst); break;
				case vk::DescriptorType::eSampler: src->descriptor.write_sampler(dst); break;
				case vk::DescriptorType::eCombinedImageSampler: src->descriptor.write_combined_image_sampler(dst); break;
				case vk::DescriptorType::eStorageImage: src->descriptor.write_storage_image(dst); break;
				case vk::DescriptorType::eUniformBufferDynamic: src->descriptor.write_uniform_buffer(dst); break;
				case vk::DescriptorType::eStorageBuffer: src->descriptor.write_buffer(dst); break;
				case vk::DescriptorType::eUniformTexelBuffer: src->descriptor.write_buffer(dst); break;
				case vk::DescriptorType::eStorageTexelBuffer: src->descriptor.write_buffer(dst); break;
				case vk::DescriptorType::eAccelerationStructureKHR: src->descriptor.write_acceleration(dst); break;
				default: break;
			}
		}

		API->m_device.updateDescriptorSets(count, writes, 0, nullptr);
		return set;
	}

	VulkanDescriptorSetAllocator* VulkanDescriptorSetAllocator::instance()
	{
		static thread_local VulkanDescriptorSetAllocator* allocator = trx_new VulkanDescriptorSetAllocator();
		return allocator;
	}
}// namespace Engine
