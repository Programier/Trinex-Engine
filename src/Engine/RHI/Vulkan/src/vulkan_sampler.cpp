#include <Graphics/sampler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_types.hpp>

namespace Engine
{

    VulkanSamplerCreateInfo::VulkanSamplerCreateInfo() = default;

    VulkanSamplerCreateInfo::VulkanSamplerCreateInfo(const SamplerCreateInfo& info)
        : wrap_s(get_type(info.wrap_s)), wrap_t(get_type(info.wrap_t)), wrap_r(get_type(info.wrap_r)),
          compare_func(get_type(info.compare_func)), anisotropy(info.anisotropy), mip_lod_bias(info.mip_lod_bias),
          min_lod(info.min_lod), max_lod(info.max_lod), unnormalized_coordinates(info.unnormalized_coordinates),
          compare_enable(info.compare_mode == CompareMode::RefToTexture)
    {
        switch (info.filter)
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
        vk::SamplerCreateInfo sampler_info({}, info.mag_filter, info.min_filter, info.mipmap_mode, info.wrap_s,
                                           info.wrap_t, info.wrap_r, info.mip_lod_bias,
                                           static_cast<vk::Bool32>(info.anisotropy > 1.0), info.anisotropy,
                                           info.compare_enable, info.compare_func, info.min_lod, info.max_lod,
                                           vk::BorderColor::eIntOpaqueBlack, info.unnormalized_coordinates);

        destroy();
        _M_sampler = API->_M_device.createSampler(sampler_info);
        return *this;
    }

    VulkanSampler& VulkanSampler::destroy()
    {
        API->wait_idle();
        DESTROY_CALL(destroySampler, _M_sampler);
        return *this;
    }

    void VulkanSampler::bind(BindingIndex binding, BindingIndex set)
    {}


    VulkanSampler::~VulkanSampler()
    {
        destroy();
    }


    RHI_Sampler* VulkanAPI::create_sampler(const SamplerCreateInfo& info)
    {
        return &(new VulkanSampler())->create(info);
    }
}// namespace Engine
