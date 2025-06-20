#include <Core/default_resources.hpp>
#include <Graphics/sampler.hpp>
#include <RHI/rhi_initializers.hpp>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	static vk::BorderColor parse_border_color(Color color)
	{
		const byte color_intensity = (color.r + color.g + color.b) / 3;
		const byte alpha_intensity = color.a;

		if (alpha_intensity <= 127)
			return vk::BorderColor::eFloatTransparentBlack;
		return color_intensity > 127 ? vk::BorderColor::eFloatOpaqueWhite : vk::BorderColor::eFloatOpaqueBlack;
	}

	VulkanSampler& VulkanSampler::create(const RHISamplerInitializer* initializer)
	{
		vk::SamplerCreateInfo sampler_info;
		sampler_info.addressModeU     = VulkanEnums::address_mode_of(initializer->address_u);
		sampler_info.addressModeV     = VulkanEnums::address_mode_of(initializer->address_v);
		sampler_info.addressModeW     = VulkanEnums::address_mode_of(initializer->address_w);
		sampler_info.compareOp        = VulkanEnums::compare_of(initializer->compare_func);
		sampler_info.mipLodBias       = initializer->mip_lod_bias;
		sampler_info.anisotropyEnable = initializer->anisotropy > 1.f;
		sampler_info.maxAnisotropy    = initializer->anisotropy;
		sampler_info.compareEnable    = initializer->compare_func != RHICompareFunc::Never;
		sampler_info.minLod           = initializer->min_lod;
		sampler_info.maxLod           = initializer->max_lod;

		switch (initializer->filter)
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

		vk::SamplerCustomBorderColorCreateInfoEXT border_color;

		if (API->is_extension_enabled(VK_EXT_CUSTOM_BORDER_COLOR_EXTENSION_NAME))
		{
			sampler_info.borderColor = vk::BorderColor::eFloatCustomEXT;
			border_color.format      = vk::Format::eR32G32B32A32Sfloat;
			for (int i = 0; i < 4; ++i)
				border_color.customBorderColor.float32[i] = static_cast<float>(initializer->border_color[i]) / 255.f;
			sampler_info.setPNext(&border_color);
		}
		else
		{
			sampler_info.borderColor = parse_border_color(initializer->border_color);
		}

		m_sampler = API->m_device.createSampler(sampler_info);
		return *this;
	}

	VulkanSampler::~VulkanSampler()
	{
		DESTROY_CALL(destroySampler, m_sampler);
	}

	VulkanAPI& VulkanAPI::bind_sampler(RHI_Sampler* sampler, byte slot)
	{
		m_state_manager->samplers.bind(static_cast<VulkanSampler*>(sampler)->sampler(), slot);
		return *this;
	}

	RHI_Sampler* VulkanAPI::create_sampler(const RHISamplerInitializer* sampler)
	{
		return &(new VulkanSampler())->create(sampler);
	}
}// namespace Engine
