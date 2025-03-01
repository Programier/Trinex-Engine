#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_descript_set_layout.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanDescriptorPool;
	struct VulkanDescriptorSet;
	struct VulkanSampler;
	struct VulkanTexture;
	struct VulkanDescriptorSetLayout;

	struct VulkanPipeline : public RHI_DefaultDestroyable<RHI_Pipeline> {
		struct State {
			vk::PipelineInputAssemblyStateCreateInfo input_assembly;
			vk::PipelineRasterizationStateCreateInfo rasterizer;
			vk::PipelineMultisampleStateCreateInfo multisampling;
			vk::PipelineDepthStencilStateCreateInfo depth_stencil;
			Vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment;
			vk::PipelineColorBlendStateCreateInfo color_blending;
			vk::SampleMask sample_mask;
			vk::PipelineDynamicStateCreateInfo dynamic_state_info;

			State& init(const GraphicsPipeline* m_engine_pipeline, bool with_flipped_viewport);
		};

		const GraphicsPipeline* m_engine_pipeline;
		VulkanDescriptorSetLayout* m_descriptor_set_layout;
		vk::PipelineLayout m_pipeline_layout;
		TreeMap<Identifier, vk::Pipeline> m_pipelines;
		VulkanDescriptorSet* m_descriptor_set = nullptr;

		State& create_pipeline_state(bool with_flipped_viewport);
		VulkanPipeline& create_descriptor_set_layout();
		Vector<vk::PipelineShaderStageCreateInfo> create_pipeline_stage_infos();
		vk::PipelineVertexInputStateCreateInfo create_vertex_input_info();
		bool create_pipeline_layout();
		vk::Pipeline find_or_create_pipeline();

		bool create(const GraphicsPipeline* pipeline);
		VulkanDescriptorSet* current_descriptor_set();
		void bind() override;

		VulkanPipeline& bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location);
		VulkanPipeline& bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location, vk::DescriptorType type);
		VulkanPipeline& bind_sampler(VulkanSampler* sampler, BindLocation location);
		VulkanPipeline& bind_texture(VulkanTexture* texture, BindLocation location);
		VulkanPipeline& bind_texture_combined(VulkanTexture*, VulkanSampler*, BindLocation);
		VulkanPipeline& bind_descriptor_set();

		~VulkanPipeline();
	};
}// namespace Engine
