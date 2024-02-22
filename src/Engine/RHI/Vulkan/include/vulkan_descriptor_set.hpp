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


    struct VulkanDescriptorSet {

        struct CombinedImageSampler {
            struct VulkanSampler* m_sampler = nullptr;
            struct VulkanTexture* m_texture = nullptr;
        };

        vk::DescriptorSet m_set;


        BindingVariable<VulkanSSBO*> m_ssbo                            = {};
        BindingVariable<VulkanSampler*> m_sampler                      = {};
        BindingVariable<VulkanTexture*> m_texture                      = {};
        BindingVariable<CombinedImageSampler> m_combined_image_sampler = {};

        VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout);
        VulkanDescriptorSet& bind(vk::PipelineLayout& layout, BindingIndex set);
    };
}// namespace Engine
