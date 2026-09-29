#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Trinex
{
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;

	class VulkanShader : public VulkanDeferredDestroy<RHIShader>
	{
	private:
		vk::ShaderModule m_shader;

	public:
		VulkanShader(Span<u8> source, Span<RHIShaderParameterInfo> parameters);
		~VulkanShader();

		inline vk::ShaderModule module() const { return m_shader; }
	};
}// namespace Trinex
