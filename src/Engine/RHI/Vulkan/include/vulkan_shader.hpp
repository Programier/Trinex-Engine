#pragma once

#include <Core/engine_types.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{

    struct VulkanDescriptorPool;
    struct VulkanDescriptorSet;

    struct VulkanShaderBase : public RHI_Shader {
        vk::ShaderModule m_shader;

        VulkanShaderBase& create(const Shader* shader);
        VulkanShaderBase& destroy();
    };


    struct VulkanVertexShader : public VulkanShaderBase {
        Vector<vk::VertexInputBindingDescription> m_binding_description;
        Vector<vk::VertexInputAttributeDescription> m_attribute_description;

        VulkanVertexShader& create(const VertexShader* shader);
        VulkanVertexShader& destroy();

        ~VulkanVertexShader();
    };

    struct VulkanTessellationControlShader : public VulkanShaderBase {
        VulkanTessellationControlShader& create(const TessellationControlShader* shader);
        VulkanTessellationControlShader& destroy();

        ~VulkanTessellationControlShader();
    };

    struct VulkanTessellationShader : public VulkanShaderBase {
        VulkanTessellationShader& create(const TessellationShader* shader);
        VulkanTessellationShader& destroy();

        ~VulkanTessellationShader();
    };

    struct VulkanGeometryShader : public VulkanShaderBase {
        VulkanGeometryShader& create(const GeometryShader* shader);
        VulkanGeometryShader& destroy();

        ~VulkanGeometryShader();
    };

    struct VulkanFragmentShader : public VulkanShaderBase {
        VulkanFragmentShader& create(const FragmentShader* shader);
        VulkanFragmentShader& destroy();

        ~VulkanFragmentShader();
    };
}// namespace Engine
