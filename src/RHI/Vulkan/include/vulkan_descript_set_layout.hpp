#pragma once
#include <Core/engine_types.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanDescriptorSetLayout {
        vk::DescriptorSetLayout layout = {};

        byte uniform_buffers        = 0;
        byte textures               = 0;
        byte samplers               = 0;
        byte combined_image_sampler = 0;

        FORCE_INLINE bool has_layouts() const
        {
            return layout != 0;
        }

        void destroy();
        ~VulkanDescriptorSetLayout();
    };
}// namespace Engine
