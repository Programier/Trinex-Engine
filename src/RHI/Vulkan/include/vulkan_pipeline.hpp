#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;
	class VulkanSampler;
	class VulkanSRV;
	class VulkanUAV;
	struct VulkanDescriptorSetLayout;
	class Pipeline;

	struct VulkanPipeline : public VulkanDeferredDestroy<RHI_Pipeline> {
		VulkanDescriptorSetLayout* m_descriptor_set_layout;
		vk::PipelineLayout m_pipeline_layout;
		VulkanDescriptorSet* m_descriptor_set = nullptr;

		VulkanPipeline& create_descriptor_set_layout(const Pipeline* pipeline);
		VulkanDescriptorSet* current_descriptor_set();
		bool create_pipeline_layout();

		VulkanPipeline& bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location, vk::DescriptorType type);
		VulkanPipeline& bind_sampler(VulkanSampler* sampler, BindLocation location);
		VulkanPipeline& bind_srv(VulkanSRV* srv, byte location, VulkanSampler* sampler);
		VulkanPipeline& bind_uav(VulkanUAV* uav, byte location);

		VulkanPipeline& bind_descriptor_set(vk::PipelineBindPoint point);

		~VulkanPipeline();
	};

	struct VulkanGraphicsPipeline : VulkanPipeline {
		struct State {
			vk::PipelineInputAssemblyStateCreateInfo input_assembly;
			vk::PipelineRasterizationStateCreateInfo rasterizer;
			vk::PipelineMultisampleStateCreateInfo multisampling;
			vk::PipelineDepthStencilStateCreateInfo depth_stencil;
			vk::PipelineColorBlendAttachmentState color_blend_attachment[4];
			vk::PipelineColorBlendStateCreateInfo color_blending;
			vk::SampleMask sample_mask;
			Vector<vk::PipelineShaderStageCreateInfo> stages;

			vk::PipelineVertexInputStateCreateInfo vertex_input;

			State& init(const GraphicsPipeline* m_engine_pipeline);
			State& flip_viewport();
		};

		State m_state;
		TreeMap<Identifier, vk::Pipeline> m_pipelines;

		vk::Pipeline find_or_create_pipeline();
		bool create(const GraphicsPipeline* pipeline);
		void bind() override;
		~VulkanGraphicsPipeline();
	};

	struct VulkanComputePipeline : VulkanPipeline {
		vk::Pipeline m_pipeline;

		bool create(const ComputePipeline* pipeline);
		void bind() override;
		~VulkanComputePipeline();
	};
}// namespace Engine
