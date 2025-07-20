#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	VulkanShader::VulkanShader(const byte* shader, size_t size)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), size, reinterpret_cast<const uint32_t*>(shader));
		m_shader = API->m_device.createShaderModule(info);
	}

	VulkanShader::~VulkanShader(){DESTROY_CALL(destroyShaderModule, m_shader)}

	RHIShader* VulkanAPI::create_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}

}// namespace Engine
