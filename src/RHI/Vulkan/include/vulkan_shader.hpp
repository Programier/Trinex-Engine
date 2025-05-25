#pragma once

#include <Core/engine_types.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{

	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;

	class VulkanShader : public RHI_DefaultDestroyable<RHI_Shader>
	{
	private:
		vk::ShaderModule m_shader;

	public:
		VulkanShader(const byte* shader, size_t size);
		~VulkanShader();

		inline vk::ShaderModule module() const { return m_shader; }
	};

	class VulkanVertexShader : public VulkanShader
	{
	private:
		Vector<vk::VertexInputBindingDescription> m_binding_description;
		Vector<vk::VertexInputAttributeDescription> m_attribute_description;

	public:
		VulkanVertexShader(const byte* shader, size_t size, const VertexAttribute* attribute, size_t attributes_count);

		inline const Vector<vk::VertexInputBindingDescription>& binding_description() const { return m_binding_description; }
		inline const Vector<vk::VertexInputAttributeDescription>& attribute_description() const
		{
			return m_attribute_description;
		}
	};

	using VulkanTessellationControlShader = VulkanShader;
	using VulkanTessellationShader        = VulkanShader;
	using VulkanGeometryShader            = VulkanShader;
	using VulkanFragmentShader            = VulkanShader;
	using VulkanComputeShader             = VulkanShader;
}// namespace Engine
