#pragma once
#include <cstring>

namespace Engine
{

    struct VulkanState {
        struct VulkanFramebuffer* _M_framebuffer = nullptr;
        struct VulkanShader* _M_shader           = nullptr;
        struct VulkanPipeline* _M_pipeline       = nullptr;

        inline void reset()
        {
            std::memset(this, 0, sizeof(VulkanState));
        }
    };
}// namespace Engine
