#include <vulkan_api.hpp>
#include <vulkan_shader.hpp>

namespace Trinex
{
	VulkanShader::VulkanShader(const u8* shader, usize size)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), size, reinterpret_cast<const u32*>(shader));
		m_shader = vk::check_result(VulkanAPI::instance()->m_device.createShaderModule(info));
	}

	VulkanShader::~VulkanShader(){DESTROY_CALL(destroyShaderModule, m_shader)}

	RHIShader* VulkanAPI::create_shader(const u8* shader, usize size)
	{
		return trx_new VulkanShader(shader, size);
	}

}// namespace Trinex
