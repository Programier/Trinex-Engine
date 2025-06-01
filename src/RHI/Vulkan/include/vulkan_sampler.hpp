#pragma once

#include <Graphics/rhi.hpp>
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

		VulkanSamplerCreateInfo();
		VulkanSamplerCreateInfo(const SamplerInitializer* sampler);
	};

	class VulkanSampler : public VulkanDeferredDestroy<RHI_Sampler>
	{
	private:
		vk::Sampler m_sampler;

	public:
		VulkanSampler& create(const VulkanSamplerCreateInfo&);
		void bind(BindLocation location) override;
		inline vk::Sampler sampler() const { return m_sampler; }
		~VulkanSampler();
	};
}// namespace Engine
