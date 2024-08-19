#include <vulkan_api.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
	UniformBufferPoolBase::BufferEntry& UniformBufferPoolBase::BufferEntry::update(const void* data, size_t size, size_t offset)
	{
		byte* mapped = reinterpret_cast<byte*>(API->m_device.mapMemory(memory, 0, VK_WHOLE_SIZE));
		std::memcpy(mapped + offset, data, size);
		API->m_device.flushMappedMemoryRanges(vk::MappedMemoryRange(memory, 0, VK_WHOLE_SIZE));
		API->m_device.unmapMemory(memory);
		return *this;
	}

	UniformBufferPoolBase& UniformBufferPoolBase::allocate_new(size_t size)
	{
		buffers.emplace_back();
		auto& buffer = buffers.back();
		API->create_buffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
						   vk::MemoryPropertyFlagBits::eHostVisible, buffer.buffer, buffer.memory);

		buffer.size = size;
		return *this;
	}

	UniformBufferPoolBase::~UniformBufferPoolBase()
	{
		for (BufferEntry& entry : buffers)
		{
			DESTROY_CALL(destroyBuffer, entry.buffer);
			DESTROY_CALL(freeMemory, entry.memory);
		}

		buffers.clear();
	}

	void GlobalUniformBufferPool::push(const GlobalShaderParameters* params)
	{
		++index;
		if (index >= static_cast<int64_t>(buffers.size()))
			allocate_new();

		buffers[index].update(params, sizeof(GlobalShaderParameters), 0);
	}

	void GlobalUniformBufferPool::pop()
	{
		if (index > -1)
		{
			--index;
		}
		else
		{
			error_log("VulkanAPI", "Cannot pop global variables, because stack is already empty!");
		}
	}

	void GlobalUniformBufferPool::bind()
	{
		VulkanPipeline* pipeline = API->m_state.m_pipeline;
		if (index >= 0 && pipeline && pipeline->global_parameters_info().has_parameters())
		{
			API->m_state.m_pipeline->bind_uniform_buffer(
					vk::DescriptorBufferInfo(buffers[index].buffer, 0, sizeof(GlobalShaderParameters)),
					pipeline->global_parameters_info().bind_index(), vk::DescriptorType::eUniformBuffer);
		}
	}

	void GlobalUniformBufferPool::reset()
	{
		index = -1;
	}

	LocalUniformBufferPool::LocalUniformBufferPool()
	{
		allocate_new();
	}

	void LocalUniformBufferPool::bind()
	{
		if (shadow_data_size == 0)
			return;

		VulkanPipeline* pipeline = API->m_state.m_pipeline;

		if (pipeline && pipeline->local_parameters_info().has_parameters())
		{
			if (buffers[index].size < used_data + shadow_data_size)
			{
				++index;
				used_data = 0;

				if (buffers.size() <= index)
				{
					allocate_new(shadow_data_size);
				}
			}

			auto& current_buffer = buffers[index];
			current_buffer.update(shadow_data.data(), shadow_data_size, used_data);

			BindLocation local_params_location = pipeline->local_parameters_info().bind_index();
			API->m_state.m_pipeline->bind_uniform_buffer(
					vk::DescriptorBufferInfo(current_buffer.buffer, used_data, shadow_data_size), local_params_location,
					vk::DescriptorType::eUniformBuffer);
			used_data = align_memory(used_data + shadow_data_size, API->m_properties.limits.minUniformBufferOffsetAlignment);
		}

		shadow_data_size = 0;
	}

	void LocalUniformBufferPool::update(const void* data, size_t size, size_t offset)
	{
		shadow_data_size = glm::max(size + offset, shadow_data_size);

		if (shadow_data.size() < shadow_data_size)
		{
			shadow_data.resize(shadow_data_size);
		}

		std::memcpy(shadow_data.data() + offset, data, size);
	}

	void LocalUniformBufferPool::reset()
	{
		shadow_data_size = 0;
		index			 = 0;
		used_data		 = 0;
	}

	void VulkanUniformBuffer::reset()
	{
		global_pool.reset();
		local_pool.reset();
	}

	void VulkanUniformBuffer::bind()
	{
		global_pool.bind();
		local_pool.bind();
	}


	VulkanAPI& VulkanAPI::push_global_params(const GlobalShaderParameters& params)
	{
		API->uniform_buffer()->global_pool.push(&params);
		return *this;
	}

	VulkanAPI& VulkanAPI::pop_global_params()
	{
		API->uniform_buffer()->global_pool.pop();
		return *this;
	}

	VulkanAPI& VulkanAPI::update_local_parameter(const void* data, size_t size, size_t offset)
	{
		API->uniform_buffer()->local_pool.update(data, size, offset);
		return *this;
	}
}// namespace Engine
