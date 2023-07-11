#pragma once
#include <Core/engine_types.hpp>
#include <vulkan/vulkan.hpp>

namespace Engine
{
    template<typename Type>
    using BindingVariable = Map<BindingIndex, Type>;

    struct VulkanDescriptorSet {
        vk::DescriptorSet _M_set;


        BindingVariable<vk::Sampler> _M_sampler;
        BindingVariable<vk::ImageView> _M_image_view;
        BindingVariable<struct VulkanUniformBuffer*> _M_current_ubo;


        VulkanDescriptorSet(vk::DescriptorPool& pool, vk::DescriptorSetLayout* layout);
        VulkanDescriptorSet& bind(vk::PipelineLayout& layout);
    };
}// namespace Engine
