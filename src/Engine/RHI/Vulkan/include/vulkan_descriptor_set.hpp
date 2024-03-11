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
        vk::DescriptorSet m_set;

        BindingVariable<VulkanSSBO*> m_ssbo       = {};
        BindingVariable<VulkanSampler*> m_sampler = {};
        BindingVariable<VulkanTexture*> m_texture = {};

        VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout);
        VulkanDescriptorSet& bind(vk::PipelineLayout& layout, BindingIndex set);
    };
}// namespace Engine
