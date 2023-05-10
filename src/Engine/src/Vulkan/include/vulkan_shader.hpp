#pragma once

#include <Core/engine_types.hpp>
#include <Core/shader_types.hpp>

#include <vulkan_object.hpp>

namespace Engine
{

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
        Index _M_current_descriptor_index        = 0;
        Index _M_current_binded_descriptor_index = 0;

        uint32_t _M_last_frame           = 0;
        uint32_t _M_max_descriptors_sets = 1;

        vk::PipelineLayout _M_pipeline_layout;
        bool _M_has_descriptors = false;

        vk::DescriptorSetLayout* _M_descriptor_set_layout = nullptr;
        Vector<Vector<DescriptorSetInfo>> _M_descriptor_sets;

        vk::DescriptorPool _M_descriptor_pool;
        vk::Pipeline _M_pipeline;


        VulkanShader();

    private:
        VulkanShader& create_descriptor_layout(const PipelineCreateInfo& info);
        VulkanShader& create_descriptor_sets();
        Vector<vk::VertexInputBindingDescription> get_binding_description(const PipelineCreateInfo& info);
        Vector<vk::VertexInputAttributeDescription> get_attribute_description(const PipelineCreateInfo& info);
        Vector<vk::DescriptorPoolSize> create_pool_size(const PipelineCreateInfo& info);
        DescriptorSetInfo& current_descriptor_set();
        VulkanShader& create_new_descriptor_set(uint32_t buffer_index);

    public:
        VulkanShader& update_descriptor_layout(bool force = false);
        bool init(const PipelineCreateInfo& params);
        VulkanShader& use();
        VulkanShader& bind_ubo(struct VulkanUniformBuffer* ubo, BindingIndex binding, size_t offset, size_t size);
        VulkanShader& bind_texture(struct VulkanTexture* texture, uint_t binding);
        VulkanShader& bind_shared_buffer(class VulkanSSBO* ssbo, size_t offset, size_t size, uint_t binding);
        ~VulkanShader();
    };
}// namespace Engine
