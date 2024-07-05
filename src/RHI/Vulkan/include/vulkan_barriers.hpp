#pragma once
#include <vulkan_headers.hpp>

namespace Engine
{
    struct VulkanTexture;
}
namespace Engine::Barrier
{
    void from_compute_to_compute();
    void from_compute_to_compute(vk::CommandBuffer& cmd);
    void from_compute_to_graphics();
    void from_compute_to_graphics(vk::CommandBuffer& cmd);
    void from_graphics_to_compute();
    void from_graphics_to_compute(vk::CommandBuffer& cmd, VulkanTexture* texture);
    void from_graphics_to_transfer(VulkanTexture* texture);
    void from_graphics_to_transfer(vk::CommandBuffer& cmd, VulkanTexture* texture);

    struct LayoutFlags {
        vk::AccessFlags access;
        vk::PipelineStageFlags stage;

        void setup(vk::ImageLayout layout, bool is_swapchain_image = false);
    };

    struct ImageBarrierFlags {
        LayoutFlags src;
        LayoutFlags dst;
    };

    void transition_image_layout(vk::ImageMemoryBarrier& barrier, bool is_swapchain_image = false);
    void transition_image_layout(vk::CommandBuffer& cmd, vk::ImageMemoryBarrier& barrier, bool is_swapchain_image = false);
}// namespace Engine::Barrier
