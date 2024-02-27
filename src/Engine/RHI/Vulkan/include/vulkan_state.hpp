#pragma once
#include <cstring>

namespace Engine
{

    struct VulkanState {
        struct {
            struct VulkanRenderTargetFrame* m_framebuffer = nullptr;
            struct VulkanRenderPass* m_render_pass        = nullptr;
        } m_render_target;
        struct VulkanPipeline* m_pipeline = nullptr;
        struct VulkanVertexBuffer* m_current_vertex_buffer[15];
        struct VulkanIndexBuffer* m_current_index_buffer;
        struct VulkanViewport* m_current_viewport = nullptr;
        bool m_is_image_rendered_to_swapchain     = false;

        inline void reset()
        {
            std::memset(reinterpret_cast<void*>(this), 0, sizeof(VulkanState));
        }
    };
}// namespace Engine
