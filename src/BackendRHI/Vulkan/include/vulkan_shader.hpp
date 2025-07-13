#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;

	class VulkanShader : public VulkanDeferredDestroy<RHI_Shader>
	{
	private:
		vk::ShaderModule m_shader;

	public:
		VulkanShader(const byte* shader, size_t size);
		~VulkanShader();

		inline vk::ShaderModule module() const { return m_shader; }
	};
}// namespace Engine
