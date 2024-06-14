#pragma once
#include <Core/structures.hpp>
#include <cstring>

namespace Engine
{

    struct VulkanState {
        struct VulkanRenderTargetBase* m_render_target = nullptr;
        struct VulkanPipeline* m_pipeline              = nullptr;
        struct RHI_VertexBuffer* m_current_vertex_buffer[15];
        struct VulkanIndexBuffer* m_current_index_buffer;
        struct VulkanViewport* m_current_viewport = nullptr;
        ViewPort m_viewport;

        inline void reset()
        {
            std::memset(reinterpret_cast<void*>(this), 0, sizeof(VulkanState));
        }
    };
}// namespace Engine
