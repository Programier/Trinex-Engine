#pragma once
#include <RHI/handles.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct RHIRayTracingAccelerationInputs;
	class VulkanBuffer;

	class VulkanAccelerationStructure : public VulkanDeferredDestroy<RHIAccelerationStructure>
	{
	private:
		vk::AccelerationStructureKHR m_acceleration;
		VulkanBuffer* m_acceleration_buffer;
		size_t m_scratch_size = 0;

	public:
		VulkanAccelerationStructure(const RHIRayTracingAccelerationInputs* inputs);
		~VulkanAccelerationStructure();
	};
}// namespace Engine
