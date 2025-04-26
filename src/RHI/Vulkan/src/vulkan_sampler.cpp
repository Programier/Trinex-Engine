#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{

	VulkanSamplerCreateInfo::VulkanSamplerCreateInfo() = default;

	VulkanSamplerCreateInfo::VulkanSamplerCreateInfo(const SamplerInitializer* sampler)
	    : address_u(get_type(sampler->address_u)), address_v(get_type(sampler->address_v)),
	      address_w(get_type(sampler->address_w)), compare_func(get_type(sampler->compare_func)), anisotropy(sampler->anisotropy),
	      mip_lod_bias(sampler->mip_lod_bias), min_lod(sampler->min_lod), max_lod(sampler->max_lod),
	      compare_enable(sampler->compare_mode == CompareMode::RefToTexture)
	{
		switch (sampler->filter)
		{
			case SamplerFilter::Trilinear:
				mag_filter  = vk::Filter::eLinear;
				min_filter  = vk::Filter::eLinear;
				mipmap_mode = vk::SamplerMipmapMode::eLinear;
				break;

			case SamplerFilter::Bilinear:
				mag_filter  = vk::Filter::eLinear;
				min_filter  = vk::Filter::eLinear;
				mipmap_mode = vk::SamplerMipmapMode::eNearest;
				break;

			case SamplerFilter::Point:
				mag_filter  = vk::Filter::eNearest;
				min_filter  = vk::Filter::eNearest;
				mipmap_mode = vk::SamplerMipmapMode::eNearest;
				break;

			default:
				mag_filter  = vk::Filter::eNearest;
				min_filter  = vk::Filter::eNearest;
				mipmap_mode = vk::SamplerMipmapMode::eNearest;
		}

		border_color = sampler->border_color;
	}

	static vk::BorderColor parse_border_color(Color color)
	{
		const byte color_intensity = (color.r + color.g + color.b) / 3;
		const byte alpha_intensity = color.a;

		if (alpha_intensity <= 127)
			return vk::BorderColor::eFloatTransparentBlack;
		return color_intensity > 127 ? vk::BorderColor::eFloatOpaqueWhite : vk::BorderColor::eFloatOpaqueBlack;
	}

	VulkanSampler& VulkanSampler::create(const VulkanSamplerCreateInfo& info)
	{
		vk::SamplerCreateInfo sampler_info({}, info.mag_filter, info.min_filter, info.mipmap_mode, info.address_u, info.address_v,
		                                   info.address_w, info.mip_lod_bias, static_cast<vk::Bool32>(info.anisotropy > 1.0),
		                                   info.anisotropy, info.compare_enable, info.compare_func, info.min_lod, info.max_lod,
		                                   parse_border_color(info.border_color), vk::False);

		vk::SamplerCustomBorderColorCreateInfoEXT border_color;

		if (API->is_extension_enabled(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
		{
			sampler_info.borderColor = vk::BorderColor::eFloatCustomEXT;
			border_color.format      = vk::Format::eR32G32B32A32Sfloat;
			for (int i = 0; i < 4; ++i)
				border_color.customBorderColor.float32[i] = static_cast<float>(info.border_color[i]) / 255.f;
			sampler_info.setPNext(&border_color);
		}

		m_sampler = API->m_device.createSampler(sampler_info);
		return *this;
	}

	void VulkanSampler::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			API->m_state.m_pipeline->bind_sampler(this, location);
		}
	}

	VulkanSampler::~VulkanSampler()
	{
		DESTROY_CALL(destroySampler, m_sampler);
	}

	RHI_Sampler* VulkanAPI::create_sampler(const SamplerInitializer* sampler)
	{
		return &(new VulkanSampler())->create(sampler);
	}
}// namespace Engine

VkSampler trinex_default_vulkan_sampler(Engine::Sampler* sampler)
{
	return sampler->rhi_sampler()->as<Engine::VulkanSampler>()->m_sampler;
}
