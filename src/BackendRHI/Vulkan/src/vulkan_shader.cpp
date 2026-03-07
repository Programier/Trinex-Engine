#include <Core/logger.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>

namespace Trinex
{
	VulkanShader::VulkanShader(const u8* shader, usize size)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), size, reinterpret_cast<const u32*>(shader));
		m_shader = vk::check_result(API->m_device.createShaderModule(info));
	}

	VulkanShader::~VulkanShader(){DESTROY_CALL(destroyShaderModule, m_shader)}

	RHIShader* VulkanAPI::create_shader(const u8* shader, usize size)
	{
		return trx_new VulkanShader(shader, size);
	}

}// namespace Trinex
