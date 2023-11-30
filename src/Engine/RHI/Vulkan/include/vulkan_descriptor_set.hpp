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
            struct VulkanSampler* _M_sampler = nullptr;
            struct VulkanTexture* _M_texture = nullptr;
        };

        vk::DescriptorSet _M_set;


        BindingVariable<VulkanSSBO*> _M_ssbo                            = {};
        BindingVariable<VulkanSampler*> _M_sampler                      = {};
        BindingVariable<VulkanTexture*> _M_texture                      = {};
        BindingVariable<CombinedImageSampler> _M_combined_image_sampler = {};
        BindingVariable<struct VulkanUniformBuffer*> _M_current_ubo     = {};


        VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout);
        VulkanDescriptorSet& bind(vk::PipelineLayout& layout, BindingIndex set);
    };
}// namespace Engine
