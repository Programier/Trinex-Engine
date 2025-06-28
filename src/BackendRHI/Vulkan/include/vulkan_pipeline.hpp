#pragma once
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanPipelineLayout;
	struct VulkanDescriptorSet;
	class VulkanSampler;
	class VulkanSRV;
	class VulkanUAV;
	class VulkanStateManager;
	struct VulkanDescriptorSetLayout;
	class Pipeline;

	class VulkanPipeline : public VulkanDeferredDestroy<RHI_Pipeline>
	{
	private:
		VulkanPipelineLayout* m_layout;

		bool is_dirty_state(VulkanStateManager* manager) const;

	public:
		VulkanPipeline(const RHIShaderParameterInfo* parameter, size_t count, vk::ShaderStageFlags stages);
		VulkanPipeline& flush_descriptors(VulkanStateManager* manager, vk::PipelineBindPoint);
		virtual VulkanPipeline& flush(VulkanStateManager* manager) = 0;
		inline VulkanPipelineLayout* layout() const { return m_layout; }

		~VulkanPipeline();
	};

	class VulkanGraphicsPipeline : public VulkanPipeline
	{
	private:
		struct Key {
			class VulkanRenderPass* pass;
			vk::PrimitiveTopology primitive_topology;
			vk::PolygonMode polygon_mode;
			vk::CullModeFlags cull_mode;
			vk::FrontFace front_face;

			inline bool operator==(const Key& key) const
			{
				return pass == key.pass && primitive_topology == key.primitive_topology && polygon_mode == key.polygon_mode &&
				       cull_mode == key.cull_mode && front_face == key.front_face;
			}
		};

		struct KeyHasher {
			uint64_t operator()(const Key& key) const;
		};

		vk::PipelineInputAssemblyStateCreateInfo m_input_assembly;
		vk::PipelineRasterizationStateCreateInfo m_rasterizer;
		vk::PipelineMultisampleStateCreateInfo m_multisampling;
		vk::PipelineDepthStencilStateCreateInfo m_depth_stencil;
		vk::PipelineColorBlendAttachmentState m_color_blend_attachment[4];
		vk::PipelineColorBlendStateCreateInfo m_color_blending;
		vk::SampleMask m_sample_mask;
		Vector<vk::PipelineShaderStageCreateInfo> m_stages;
		vk::PipelineVertexInputStateCreateInfo m_vertex_input;

		Map<Key, vk::Pipeline, KeyHasher> m_pipelines;

		vk::Pipeline find_or_create_pipeline(VulkanStateManager* manager);
		
		bool is_dirty_vertex_strides(VulkanStateManager* manager);
		
	public:
		VulkanGraphicsPipeline(const RHIGraphicsPipelineInitializer* pipeline);
		void bind() override;
		VulkanPipeline& flush(VulkanStateManager* manager) override;

		~VulkanGraphicsPipeline();
	};

	class VulkanComputePipeline : public VulkanPipeline
	{
	private:
		vk::Pipeline m_pipeline;

	public:
		VulkanComputePipeline(const RHIComputePipelineInitializer* pipeline);
		void bind() override;
		VulkanPipeline& flush(VulkanStateManager* manager) override;
		~VulkanComputePipeline();
	};
}// namespace Engine
