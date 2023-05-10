#include <vulkan_api.hpp>
#include <vulkan_transition_image_layout.hpp>


namespace Engine
{

    TransitionImageLayout& TransitionImageLayout::execute(vk::PipelineStageFlags source_stage,
                                                          vk::AccessFlags src_access,
                                                          vk::PipelineStageFlags destination_stage,
                                                          vk::AccessFlags dst_access)
    {
        if (!image)
        {
            return *this;
        }

        vk::CommandBuffer execute_command_buffer =
                command_buffer ? *command_buffer : API->begin_single_time_command_buffer();

        vk::ImageSubresourceRange range(aspect_flags, base_mip, mip_count, base_layer, layer_count);

        vk::ImageMemoryBarrier barrier(src_access, dst_access, old_layout, new_layout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
                                       *image, range);

        execute_command_buffer.pipelineBarrier(source_stage, destination_stage, {}, {}, {}, barrier);

        if (!command_buffer)
        {
            API->end_single_time_command_buffer(execute_command_buffer);
        }
        return *this;
    }
}// namespace Engine
