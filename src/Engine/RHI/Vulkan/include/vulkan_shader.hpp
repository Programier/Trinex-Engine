#pragma once

#include <Core/engine_types.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_object.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{

    struct VulkanDescriptorPool;
    struct VulkanDescriptorSet;

    struct DescriptorSetInfo {
        vk::DescriptorSet _M_descriptor_set;
        vk::Sampler _M_sampler;
        vk::ImageView _M_image_view;
        Map<BindingIndex, struct VulkanUniformBuffer*> _M_current_ubo;


        DescriptorSetInfo() = default;
        DescriptorSetInfo(const vk::DescriptorSet& set) : _M_descriptor_set(set)
        {}
    };

    struct VulkanShader : VulkanObject {
        VulkanUniquePerFrame<VulkanDescriptorPool> _M_descriptor_pool;
        VulkanUniquePerFrame<Vector<VulkanDescriptorSet*>> _M_descriptor_sets;

        vk::DescriptorSetLayout* _M_descriptor_set_layout = nullptr;
        VulkanDescriptorSet* _M_current_set               = nullptr;
        vk::PipelineLayout _M_pipeline_layout;
        Index _M_current_descriptor_index = 0;
        uint32_t _M_last_frame            = 0;


        vk::Pipeline _M_pipeline;


        VulkanShader();

    private:
        VulkanShader& create_descriptor_layout(const PipelineCreateInfo& info);
        Vector<vk::VertexInputBindingDescription> get_binding_description(const PipelineCreateInfo& info);
        Vector<vk::VertexInputAttributeDescription> get_attribute_description(const PipelineCreateInfo& info);
        Vector<vk::DescriptorPoolSize> create_pool_size(const PipelineCreateInfo& info);
        VulkanDescriptorSet* current_descriptor_set();

    public:
        bool init(const PipelineCreateInfo& params);
        VulkanShader& use();
        VulkanShader& bind_ubo(struct VulkanUniformBuffer* ubo, BindingIndex binding);
        VulkanShader& bind_texture_combined(struct VulkanSampler* sampler, struct VulkanTexture* texture,
                                            uint_t binding);
        VulkanShader& bind_sampler(struct VulkanSampler* sampler, BindingIndex location, BindingIndex binding);
        VulkanShader& bind_shared_buffer(struct VulkanSSBO* ssbo, size_t offset, size_t size, uint_t binding);
        ~VulkanShader();
    };


    struct VulkanShaderBase : public RHI_Shader {
        vk::ShaderModule _M_shader;

        VulkanShaderBase& create(const ShaderBase* shader);
        VulkanShaderBase& destroy();
    };


    struct VulkanVertexShader : public VulkanShaderBase {
        Vector<vk::VertexInputBindingDescription> _M_binding_description;
        Vector<vk::VertexInputAttributeDescription> _M_attribute_description;

        VulkanVertexShader& create(const VertexShader* shader);
        VulkanVertexShader& destroy();

        ~VulkanVertexShader();
    };
}// namespace Engine
