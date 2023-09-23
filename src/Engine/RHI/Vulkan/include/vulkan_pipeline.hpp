#pragma once
#include <Graphics/rhi.hpp>
#include <vulkan/vulkan.hpp>


namespace Engine
{
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

        vk::Pipeline _M_pipeline;
        Vector<vk::DescriptorSetLayout> _M_descriptor_set_layout;
        vk::PipelineLayout _M_pipeline_layout;

        VulkanPipeline& create(const Pipeline* pipeline);
        VulkanPipeline& destroy();

        VulkanPipeline& create_descriptor_layout(const Pipeline* pipeline);


        void bind() override;

        ~VulkanPipeline();
    };
}// namespace Engine
