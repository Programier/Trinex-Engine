#include <vulkan_api.hpp>
#include <vulkan_shader.hpp>

namespace Trinex
{
	VulkanShader::VulkanShader(const void* data, usize size)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), size, static_cast<const u32*>(data));
		m_shader = vk::verify(VulkanAPI::device()->createShaderModule(info));
	}

	VulkanShader::~VulkanShader()
	{
		VulkanAPI::device()->destroyShaderModule(m_shader);
	}

	RHI* VulkanShader::device() const
	{
		return VulkanAPI::instance();
	}

	RHIShader* VulkanAPI::create_shader(const void* data, usize size)
	{
		return trx_new VulkanShader(data, size);
	}

} // namespace Trinex
