#pragma once

#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
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
		Color border_color;
	};

	class VulkanSampler : public VulkanDeferredDestroy<RHISampler>
	{
	private:
		vk::Sampler m_sampler;
		RHIDescriptor m_descriptor;

	public:
		VulkanSampler& create(const RHISamplerInitializer* sampler);
		RHIDescriptor descriptor() const override;
		inline vk::Sampler sampler() const { return m_sampler; }
		~VulkanSampler();
	};
}// namespace Engine
