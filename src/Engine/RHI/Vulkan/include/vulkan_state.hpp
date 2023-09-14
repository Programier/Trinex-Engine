#pragma once
#include <cstring>

namespace Engine
{

    struct VulkanState {
        struct VulkanFramebuffer* _M_framebuffer    = nullptr;
        struct VulkanShader* _M_shader              = nullptr;
        struct VulkanVertexBuffer* _M_vertex_buffer = nullptr;
        struct VulkanIndexBuffer* _M_index_buffer   = nullptr;

        inline void reset()
        {
            std::memset(this, 0, sizeof(VulkanState));
        }
    };
}// namespace Engine
