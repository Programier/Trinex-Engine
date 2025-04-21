#pragma once

#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanSamplerCreateInfo {
		vk::Filter mag_filter;
		vk::Filter min_filter;
		vk::SamplerMipmapMode mipmap_mode;

		vk::SamplerAddressMode address_u;
		vk::SamplerAddressMode address_v;
		vk::SamplerAddressMode address_w;

		vk::CompareOp compare_func;

		float anisotropy;
		float mip_lod_bias;
		float min_lod;
		float max_lod;
		bool compare_enable;

		VulkanSamplerCreateInfo();
		VulkanSamplerCreateInfo(const SamplerInitializer* sampler);
	};

	struct VulkanSampler : RHI_DefaultDestroyable<RHI_Sampler> {
		vk::Sampler m_sampler;

		VulkanSampler& create(const VulkanSamplerCreateInfo&);
		void bind(BindLocation location) override;
		~VulkanSampler();
	};
}// namespace Engine
