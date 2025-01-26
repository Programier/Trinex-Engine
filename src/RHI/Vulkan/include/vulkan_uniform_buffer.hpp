#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>

namespace Engine
{
	class VulkanDynamicUniformBuffer;

	class VulkanUniformBufferManager
	{
	private:
		Vector<VulkanDynamicUniformBuffer*> m_buffers;

	public:
		void update(const void* data, size_t size, size_t offset, BindingIndex buffer_index);
		void bind();
		void reset();
		~VulkanUniformBufferManager();
	};
}// namespace Engine
