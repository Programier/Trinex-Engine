#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_headers.hpp>
#include <vulkan_unique_per_frame.hpp>

namespace Engine
{


    struct BindedDesriptorSet {
        VulkanDescriptorSet* _M_current_set = nullptr;
        Index _M_current_descriptor_index   = 0;
    };

    struct VulkanPipeline : public RHI_Pipeline {

        struct State {
            vk::PipelineInputAssemblyStateCreateInfo input_assembly;
            vk::PipelineRasterizationStateCreateInfo rasterizer;
            vk::PipelineMultisampleStateCreateInfo multisampling;
            vk::PipelineDepthStencilStateCreateInfo depth_stencil;
            Vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment;
            vk::PipelineColorBlendStateCreateInfo color_blending;
            vk::SampleMask sample_mask;
        };

        VulkanUniquePerFrame<VulkanDescriptorPool> _M_descriptor_pool;
        VulkanUniquePerFrame<Vector<Vector<VulkanDescriptorSet*>>> _M_descriptor_sets;

        Vector<vk::DescriptorSetLayout> _M_descriptor_set_layout;
        Vector<BindedDesriptorSet> _M_binded_descriptor_sets;

        vk::Pipeline _M_pipeline;
        vk::PipelineLayout _M_pipeline_layout;

        size_t _M_last_frame = 0;

        VulkanPipeline& create(const Pipeline* pipeline);
        VulkanPipeline& destroy();
        Vector<Vector<vk::DescriptorPoolSize>> create_pool_sizes(const Pipeline* pipeline);

        VulkanPipeline& create_descriptor_layout(const Pipeline* pipeline);

        VulkanDescriptorSet* current_descriptor_set(BindingIndex set);

        void bind() override;
        VulkanPipeline& bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location);
        VulkanPipeline& bind_uniform_buffer(const vk::Buffer& buffer, size_t offset, size_t size, BindLocation location);
        VulkanPipeline& bind_combined_sampler(struct VulkanSampler* sampler, struct VulkanTexture* texture,
                                              BindLocation location);
        VulkanPipeline& bind_sampler(VulkanSampler* sampler, BindLocation location);
        VulkanPipeline& bind_texture(VulkanTexture* texture, BindLocation location);
        VulkanPipeline& increment_set_index();

        ~VulkanPipeline();
    };
}// namespace Engine
