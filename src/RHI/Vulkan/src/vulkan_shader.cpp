#include <vulkan_api.hpp>
#include <vulkan_shader.hpp>

namespace Trinex
{
	VulkanShader::VulkanShader(Span<u8> source, Span<RHIBinding> parameters)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), source.size(), reinterpret_cast<u32*>(source.data()));
		m_shader = vk::verify(VulkanAPI::device()->createShaderModule(info));
	}

	VulkanShader::~VulkanShader(){DESTROY_CALL(destroyShaderModule, m_shader)}

	RHIShader* VulkanAPI::create_shader(Span<u8> source, Span<RHIBinding> parameters)
	{
		return trx_new VulkanShader(source, parameters);
	}

} // namespace Trinex
