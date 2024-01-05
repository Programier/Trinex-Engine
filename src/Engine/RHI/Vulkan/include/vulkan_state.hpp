#pragma once
#include <cstring>

namespace Engine
{

    struct VulkanState {
        struct VulkanRenderTargetFrame* _M_framebuffer = nullptr;
        struct VulkanPipeline* _M_pipeline             = nullptr;
        struct VulkanVertexBuffer* _M_current_vertex_buffer[15];
        struct VulkanIndexBuffer* _M_current_index_buffer;
        struct VulkanViewport* _M_current_viewport = nullptr;
        bool _M_is_image_rendered_to_swapchain     = false;

        inline void reset()
        {
            std::memset(this, 0, sizeof(VulkanState));
        }
    };
}// namespace Engine
