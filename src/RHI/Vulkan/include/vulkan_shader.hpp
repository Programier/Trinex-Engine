#pragma once

#include <Core/engine_types.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{

	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;

	struct VulkanShaderBase : public RHI_DefaultDestroyable<RHI_Shader> {
		vk::ShaderModule m_shader;

		bool create(const Shader* shader);
		~VulkanShaderBase();
	};


	struct VulkanVertexShader : public VulkanShaderBase {
		Vector<vk::VertexInputBindingDescription> m_binding_description;
		Vector<vk::VertexInputAttributeDescription> m_attribute_description;

		bool create(const VertexShader* shader);
	};

	struct VulkanTessellationControlShader : public VulkanShaderBase {
	};

	struct VulkanTessellationShader : public VulkanShaderBase {
	};

	struct VulkanGeometryShader : public VulkanShaderBase {
	};

	struct VulkanFragmentShader : public VulkanShaderBase {
	};
}// namespace Engine
