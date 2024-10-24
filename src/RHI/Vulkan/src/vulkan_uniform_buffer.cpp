#include <vulkan_api.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
	UniformBufferPoolBase& UniformBufferPoolBase::allocate_new(size_t size)
	{
		VulkanBuffer* buffer = new VulkanBuffer();
		buffer->create(size, nullptr, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU);
		buffers.push_back(buffer);
		return *this;
	}

	UniformBufferPoolBase::~UniformBufferPoolBase()
	{
		for (VulkanBuffer* buffer : buffers)
		{
			buffer->release();
		}

		buffers.clear();
	}

	void GlobalUniformBufferPool::push(const GlobalShaderParameters* params)
	{
		++index;
		if (index >= static_cast<int64_t>(buffers.size()))
			allocate_new();

		buffers[index]->copy(0, reinterpret_cast<const byte*>(params), sizeof(GlobalShaderParameters));
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
			        vk::DescriptorBufferInfo(buffers[index]->m_buffer, 0, sizeof(GlobalShaderParameters)),
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
			if (buffers[index]->m_size < used_data + shadow_data_size)
			{
				++index;
				used_data = 0;

				if (buffers.size() <= index)
				{
					allocate_new(shadow_data_size);
				}
			}

			auto& current_buffer = buffers[index];
			current_buffer->copy(used_data, shadow_data.data(), shadow_data_size);

			BindLocation local_params_location = pipeline->local_parameters_info().bind_index();
			API->m_state.m_pipeline->bind_uniform_buffer(
			        vk::DescriptorBufferInfo(current_buffer->m_buffer, used_data, shadow_data_size), local_params_location,
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
		index            = 0;
		used_data        = 0;
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
