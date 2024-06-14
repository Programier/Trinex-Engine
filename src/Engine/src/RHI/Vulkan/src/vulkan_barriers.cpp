#include <Core/exception.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_texture.hpp>

namespace Engine::Barrier
{
    void from_compute_to_compute()
    {
        return from_compute_to_compute(API->current_command_buffer());
    }

    void from_compute_to_compute(vk::CommandBuffer& cmd)
    {
        // TODO
    }

    void from_compute_to_graphics()
    {
        return from_compute_to_graphics(API->current_command_buffer());
    }

    void from_compute_to_graphics(vk::CommandBuffer& cmd)
    {
        // TODO
    }

    void from_graphics_to_compute(VulkanTexture* texture)
    {
        return from_graphics_to_compute(API->current_command_buffer(), texture);
    }

    void from_graphics_to_compute(vk::CommandBuffer& cmd, VulkanTexture* texture)
    {
        // TODO
    }

    void from_graphics_to_transfer(VulkanTexture* texture, vk::ImageLayout new_layout)
    {
        return from_graphics_to_transfer(API->current_command_buffer(), texture);
    }

    void from_graphics_to_transfer(vk::CommandBuffer& cmd, VulkanTexture* texture)
    {
        // TODO
    }

    static constexpr inline vk::PipelineStageFlags all_shaders_stage =
            vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eTessellationControlShader |
            vk::PipelineStageFlagBits::eTessellationEvaluationShader | vk::PipelineStageFlagBits::eFragmentShader |
            vk::PipelineStageFlagBits::eGeometryShader;

    void LayoutFlags::setup(vk::ImageLayout layout, bool is_swapchain_image)
    {
        switch (layout)
        {
            case vk::ImageLayout::eUndefined:
                access = static_cast<vk::AccessFlagBits>(0);
                stage  = vk::PipelineStageFlagBits::eTopOfPipe;
                break;

            case vk::ImageLayout::eGeneral:
                access = vk::AccessFlagBits::eTransferWrite;
                stage  = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eColorAttachmentOptimal:
                access = vk::AccessFlagBits::eColorAttachmentWrite;
                stage  = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                break;

            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                access = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                stage  = vk::PipelineStageFlagBits::eEarlyFragmentTests;
                break;

            case vk::ImageLayout::ePresentSrcKHR:
                access = vk::AccessFlagBits::eMemoryRead;
                stage  = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eShaderReadOnlyOptimal:
                access = vk::AccessFlagBits::eShaderRead;
                stage  = all_shaders_stage;
                break;

            case vk::ImageLayout::eTransferSrcOptimal:
                access = vk::AccessFlagBits::eTransferRead;
                stage  = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eTransferDstOptimal:
                access = vk::AccessFlagBits::eTransferWrite;
                stage  = vk::PipelineStageFlagBits::eTransfer;
                break;

            case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
            case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
            case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
            case vk::ImageLayout::eDepthAttachmentOptimal:
            case vk::ImageLayout::eDepthReadOnlyOptimal:
            case vk::ImageLayout::eStencilAttachmentOptimal:
            case vk::ImageLayout::eStencilReadOnlyOptimal:
            case vk::ImageLayout::eReadOnlyOptimal:
            case vk::ImageLayout::eAttachmentOptimal:
                break;

            default:
                throw EngineException("Undefined layout");
        }
    }

    static void submit_image_layout(vk::CommandBuffer& cmd, vk::ImageMemoryBarrier& barrier, bool is_swapchain_image)
    {
        ImageBarrierFlags flags;

        flags.src.setup(barrier.oldLayout, is_swapchain_image);
        flags.dst.setup(barrier.newLayout, is_swapchain_image);

        barrier.srcAccessMask = flags.src.access;
        barrier.dstAccessMask = flags.dst.access;
        cmd.pipelineBarrier(flags.src.stage, flags.dst.stage, {}, {}, {}, barrier);
    }

    void transition_image_layout(vk::ImageMemoryBarrier& barrier, bool is_swapchain_image)
    {
        auto cmd = API->begin_single_time_command_buffer();
        transition_image_layout(cmd, barrier, is_swapchain_image);
        API->end_single_time_command_buffer(cmd);
    }

    void transition_image_layout(vk::CommandBuffer& cmd, vk::ImageMemoryBarrier& barrier, bool is_swapchain_image)
    {
        submit_image_layout(cmd, barrier, is_swapchain_image);
    }
}// namespace Engine::Barrier
