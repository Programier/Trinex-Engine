#pragma once

#include <Core/engine_types.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{

    struct VulkanDescriptorPool;
    struct VulkanDescriptorSet;

    struct VulkanShaderBase : public RHI_Shader {
        vk::ShaderModule _M_shader;

        VulkanShaderBase& create(const Shader* shader);
        VulkanShaderBase& destroy();
    };


    struct VulkanVertexShader : public VulkanShaderBase {
        Vector<vk::VertexInputBindingDescription> _M_binding_description;
        Vector<vk::VertexInputAttributeDescription> _M_attribute_description;

        VulkanVertexShader& create(const VertexShader* shader);
        VulkanVertexShader& destroy();

        ~VulkanVertexShader();
    };

    struct VulkanFragmentShader : public VulkanShaderBase {
        VulkanFragmentShader& create(const FragmentShader* shader);
        VulkanFragmentShader& destroy();

        ~VulkanFragmentShader();
    };
}// namespace Engine
