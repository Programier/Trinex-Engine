#pragma once
#include <vulkan/vulkan.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct TransitionImageLayout {
        vk::Image* image = nullptr;
        vk::CommandBuffer* command_buffer = nullptr;
        vk::ImageLayout old_layout;
        vk::ImageLayout new_layout;
        MipMapLevel base_mip = 0;
        MipMapLevel mip_count = 1;
        uint_t base_layer = 0;
        uint_t layer_count = 1;
        vk::ImageAspectFlags aspect_flags = vk::ImageAspectFlagBits::eColor;


        TransitionImageLayout& execute(vk::PipelineStageFlags source_stage, vk::AccessFlags src_access,
                     vk::PipelineStageFlags destination_stage, vk::AccessFlags dst_access);
    };
}
