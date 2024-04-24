#pragma once
#include <Core/engine_types.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    template<typename Type>
    using BindingVariable = Type[MAX_BINDING_INDEX];

    struct VulkanSampler;
    struct VulkanTexture;
    struct VulkanSSBO;


    struct VulkanCombinedImageSampler {
        VulkanTexture* texture = nullptr;
        VulkanSampler* sampler = nullptr;
    };

    struct VulkanDescriptorSet {
        vk::DescriptorSet m_set;

        BindingVariable<VulkanSSBO*> m_ssbo                                  = {};
        BindingVariable<VulkanSampler*> m_sampler                            = {};
        BindingVariable<VulkanTexture*> m_texture                            = {};
        BindingVariable<VulkanCombinedImageSampler> m_combined_image_sampler = {};

        VulkanDescriptorSet& init(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout);
        VulkanDescriptorSet& bind(vk::PipelineLayout& layout, BindingIndex set,
                                  vk::PipelineBindPoint point                     = vk::PipelineBindPoint::eGraphics,
                                  const vk::ArrayProxy<uint32_t>& dynamic_offsets = {});

        VulkanDescriptorSet& bind_ssbo(struct VulkanSSBO* ssbo, BindingIndex index);
        VulkanDescriptorSet& bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindingIndex index,
                                                 vk::DescriptorType type);
        VulkanDescriptorSet& bind_sampler(VulkanSampler* sampler, BindingIndex index);
        VulkanDescriptorSet& bind_texture(VulkanTexture* texture, BindingIndex index);
        VulkanDescriptorSet& bind_texture_combined(VulkanTexture*, VulkanSampler*, BindingIndex index);
    };
}// namespace Engine
