#pragma once

#include <vulkan_object.hpp>
#include <Core/engine_types.hpp>

namespace Engine
{
    struct VulkanShader : VulkanObject
    {
        ShaderParams _M_shader_params;
        vk::PipelineLayout _M_pipeline_layout;
        vk::Pipeline _M_pipeline;

        VulkanShader();
        void* get_instance_data() override;
        VulkanShader& init(const ShaderParams& params);

        ~VulkanShader();
    };
}
