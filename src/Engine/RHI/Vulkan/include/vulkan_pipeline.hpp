#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{
    struct VulkanDescriptorPool;
    struct VulkanDescriptorSet;
    struct VulkanSampler;
    struct VulkanTexture;

    struct VulkanPipeline : public RHI_Pipeline {

        struct State {
            vk::PipelineInputAssemblyStateCreateInfo input_assembly;
            vk::PipelineRasterizationStateCreateInfo rasterizer;
            vk::PipelineMultisampleStateCreateInfo multisampling;
            vk::PipelineDepthStencilStateCreateInfo depth_stencil;
            Vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment;
            vk::PipelineColorBlendStateCreateInfo color_blending;
            vk::SampleMask sample_mask;


            State(const Pipeline& pipeline);
        };

        const Pipeline* m_engine_pipeline;

        VulkanDescriptorPool* m_descriptor_pool;
        Vector<vk::DescriptorSetLayout> m_descriptor_set_layout;

        vk::Pipeline m_pipeline;
        vk::PipelineLayout m_pipeline_layout;

        size_t m_last_frame = 0;


        bool create(const Pipeline* pipeline);
        VulkanPipeline& destroy();
        size_t descriptor_sets_count() const;

        VulkanPipeline& create_descriptor_layout(const Pipeline* pipeline);
        Vector<vk::DescriptorPoolSize> calculate_pool_size(const Pipeline* pipeline);
        VulkanDescriptorSet* current_descriptor_set(BindingIndex set);
        const MaterialScalarParametersInfo& global_parameters_info() const;
        const MaterialScalarParametersInfo& local_parameters_info() const;

        void bind() override;
        VulkanPipeline& submit_descriptors();
        VulkanPipeline& bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location);
        VulkanPipeline& bind_uniform_buffer(const vk::Buffer& buffer, size_t offset, size_t size, BindLocation location);
        VulkanPipeline& bind_sampler(VulkanSampler* sampler, BindLocation location);
        VulkanPipeline& bind_texture(VulkanTexture* texture, BindLocation location);
        VulkanPipeline& bind_texture_combined(VulkanTexture*, VulkanSampler*, BindLocation);

        ~VulkanPipeline();
    };
}// namespace Engine
