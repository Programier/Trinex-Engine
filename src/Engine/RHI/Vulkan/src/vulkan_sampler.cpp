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

    VulkanSamplerCreateInfo::VulkanSamplerCreateInfo(const Sampler* sampler)
        : wrap_s(get_type(sampler->wrap_s)), wrap_t(get_type(sampler->wrap_t)), wrap_r(get_type(sampler->wrap_r)),
          compare_func(get_type(sampler->compare_func)), anisotropy(sampler->anisotropy), mip_lod_bias(sampler->mip_lod_bias),
          min_lod(sampler->min_lod), max_lod(sampler->max_lod), unnormalized_coordinates(sampler->unnormalized_coordinates),
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
    }

    VulkanSampler& VulkanSampler::create(const VulkanSamplerCreateInfo& info)
    {
        vk::SamplerCreateInfo sampler_info({}, info.mag_filter, info.min_filter, info.mipmap_mode, info.wrap_s, info.wrap_t,
                                           info.wrap_r, info.mip_lod_bias, static_cast<vk::Bool32>(info.anisotropy > 1.0),
                                           info.anisotropy, info.compare_enable, info.compare_func, info.min_lod, info.max_lod,
                                           vk::BorderColor::eIntOpaqueBlack, info.unnormalized_coordinates);

        destroy();
        m_sampler = API->m_device.createSampler(sampler_info);
        return *this;
    }

    VulkanSampler& VulkanSampler::destroy()
    {
        DESTROY_CALL(destroySampler, m_sampler);
        return *this;
    }

    void VulkanSampler::bind(BindLocation location)
    {
        if (API->m_state->m_pipeline)
        {
            API->m_state->m_pipeline->bind_sampler(this, location);
        }
    }


    VulkanSampler::~VulkanSampler()
    {
        destroy();
    }


    RHI_Sampler* VulkanAPI::create_sampler(const Sampler* sampler)
    {
        return &(new VulkanSampler())->create(sampler);
    }
}// namespace Engine

VkSampler trinex_default_vulkan_sampler()
{
    static VkSampler sampler = Engine::DefaultResources::default_sampler->rhi_object<Engine::VulkanSampler>()->m_sampler;
    return sampler;
}
