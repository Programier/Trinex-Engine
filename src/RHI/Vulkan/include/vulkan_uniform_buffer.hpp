#pragma once
#include <Core/memory.hpp>
#include <Graphics/shader_parameters.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct UniformBufferPoolBase {
		Vector<VulkanBuffer*> buffers;

	protected:
		UniformBufferPoolBase& allocate_new(size_t size);
		~UniformBufferPoolBase();
	};

	template<size_t pool_size>
	struct UniformBufferPool : public UniformBufferPoolBase {
		UniformBufferPool& allocate_new(size_t size = pool_size)
		{
			UniformBufferPoolBase::allocate_new(glm::max(size, pool_size));
			return *this;
		}
	};

	struct LocalUniformBufferPool : public UniformBufferPool<4096> {
		Vector<byte> shadow_data;
		size_t shadow_data_size = 0;
		size_t index            = 0;
		size_t used_data        = 0;

		LocalUniformBufferPool();

		void update(const void* data, size_t size, size_t offset);
		void bind();
		void reset();
	};

	struct VulkanUniformBufferManager {
		LocalUniformBufferPool local_pool;

		void reset();
		void bind();
	};
}// namespace Engine
