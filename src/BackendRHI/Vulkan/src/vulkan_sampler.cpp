#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/initializers.hpp>
#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_context.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_sampler.hpp>

namespace Trinex
{
	VulkanSampler& VulkanSampler::create(const RHISamplerDesc& desc)
	{
		vk::SamplerCreateInfo sampler_info;
		sampler_info.addressModeU     = VulkanEnums::address_mode_of(desc.address_u);
		sampler_info.addressModeV     = VulkanEnums::address_mode_of(desc.address_v);
		sampler_info.addressModeW     = VulkanEnums::address_mode_of(desc.address_w);
		sampler_info.compareOp        = VulkanEnums::compare_of(desc.compare_func);
		sampler_info.mipLodBias       = desc.mip_lod_bias;
		sampler_info.anisotropyEnable = desc.anisotropy > 1.f;
		sampler_info.maxAnisotropy    = desc.anisotropy;
		sampler_info.compareEnable    = desc.compare_func != RHICompareFunc::Never;
		sampler_info.minLod           = desc.min_lod;
		sampler_info.maxLod           = desc.max_lod;

		switch (desc.filter)
		{
			case RHISamplerFilter::Trilinear:
				sampler_info.magFilter  = vk::Filter::eLinear;
				sampler_info.minFilter  = vk::Filter::eLinear;
				sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
				break;

			case RHISamplerFilter::Bilinear:
				sampler_info.magFilter  = vk::Filter::eLinear;
				sampler_info.minFilter  = vk::Filter::eLinear;
				sampler_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
				break;

			case RHISamplerFilter::Point:
				sampler_info.magFilter  = vk::Filter::eNearest;
				sampler_info.minFilter  = vk::Filter::eNearest;
				sampler_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
				break;

			default:
				sampler_info.magFilter  = vk::Filter::eNearest;
				sampler_info.minFilter  = vk::Filter::eNearest;
				sampler_info.mipmapMode = vk::SamplerMipmapMode::eNearest;
		}

		sampler_info.borderColor = VulkanEnums::border_color_of(desc.border_color);

		m_sampler    = vk::check_result(VulkanAPI::instance()->m_device.createSampler(sampler_info));
		m_descriptor = VulkanAPI::instance()->descriptor_heap()->allocate(m_sampler);
		return *this;
	}

	RHIDescriptor VulkanSampler::descriptor() const
	{
		return m_descriptor;
	}

	VulkanSampler::~VulkanSampler()
	{
		VulkanAPI::instance()->descriptor_heap()->release(m_descriptor, VulkanDescriptorHeap::Sampler);
		DESTROY_CALL(destroySampler, m_sampler);
	}

	VulkanContext& VulkanContext::bind_sampler(RHISampler* sampler, u8 slot)
	{
		samplers.bind(static_cast<VulkanSampler*>(sampler)->sampler(), slot);
		return *this;
	}

	RHISampler* VulkanAPI::create_sampler(const RHISamplerDesc& desc)
	{
		return &(trx_new VulkanSampler())->create(desc);
	}
}// namespace Trinex
