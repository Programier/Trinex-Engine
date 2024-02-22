#pragma once

#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanSamplerCreateInfo {
        vk::Filter mag_filter;
        vk::Filter min_filter;
        vk::SamplerMipmapMode mipmap_mode;

        vk::SamplerAddressMode wrap_s;
        vk::SamplerAddressMode wrap_t;
        vk::SamplerAddressMode wrap_r;

        vk::CompareOp compare_func;

        float anisotropy;
        float mip_lod_bias;
        float min_lod;
        float max_lod;

        bool unnormalized_coordinates;
        bool compare_enable;

        VulkanSamplerCreateInfo();
        VulkanSamplerCreateInfo(const Sampler* sampler);
    };


    struct VulkanSampler : RHI_Sampler {
        vk::Sampler m_sampler;

        VulkanSampler& create(const VulkanSamplerCreateInfo&);
        void bind(BindLocation location) override;
        VulkanSampler& destroy();
        ~VulkanSampler();
    };
}// namespace Engine
