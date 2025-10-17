#pragma once
#include <Core/etl/flat_map.hpp>
#include <RHI/rhi.hpp>
#include <vulkan_destroyable.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	class VulkanPipelineLayout;
	class VulkanSampler;
	class VulkanSRV;
	class VulkanUAV;
	class VulkanStateManager;
	class VulkanContext;
	class VulkanBuffer;
	class Pipeline;

	struct VulkanVertexAttribute {
		vk::Format format;
		RHIVertexSemantic semantic;
		byte semantic_index;
		byte binding;
	};

	class VulkanPipeline : public VulkanDeferredDestroy<RHIPipeline>
	{
	private:
		VulkanPipelineLayout* m_layout = nullptr;

		bool is_dirty_state(VulkanStateManager* manager) const;

	protected:
		VulkanPipelineLayout* create_layout(const RHIShaderParameterInfo* parameter, size_t count, vk::ShaderStageFlags stages);

	public:
		VulkanPipeline& flush_descriptors(VulkanContext* manager, vk::PipelineBindPoint);
		virtual VulkanPipeline& flush(VulkanContext* ctx) = 0;
		inline VulkanPipelineLayout* layout() const { return m_layout; }

		~VulkanPipeline();
	};

	class VulkanGraphicsPipeline : public VulkanPipeline
	{
	private:
		VulkanVertexAttribute* m_vertex_attributes = nullptr;
		uint16_t m_vertex_attributes_count         = 0;

		vk::PipelineInputAssemblyStateCreateInfo m_input_assembly;
		vk::PipelineRasterizationStateCreateInfo m_rasterizer;
		vk::PipelineDepthStencilStateCreateInfo m_depth_stencil;
		vk::PipelineColorBlendAttachmentState m_color_blend_attachment[4];
		vk::PipelineColorBlendStateCreateInfo m_color_blending;
		Vector<vk::PipelineShaderStageCreateInfo> m_stages;

		FlatMap<Identifier, vk::Pipeline> m_pipelines;

		vk::Pipeline find_or_create_pipeline(VulkanStateManager* manager);
		bool is_dirty_vertex_input(VulkanStateManager* manager);

	public:
		VulkanGraphicsPipeline(const RHIGraphicsPipelineInitializer* pipeline);
		VulkanPipeline& flush(VulkanContext* ctx) override;
		~VulkanGraphicsPipeline();
	};

	class VulkanMeshPipeline : public VulkanPipeline
	{
	private:
		struct Key {
			class VulkanRenderPass* pass;
			vk::PolygonMode polygon_mode;
			vk::CullModeFlags cull_mode;
			vk::FrontFace front_face;
			vk::ColorComponentFlags write_mask;

			inline bool operator==(const Key& key) const
			{
				return pass == key.pass && polygon_mode == key.polygon_mode && cull_mode == key.cull_mode &&
				       front_face == key.front_face && write_mask == key.write_mask;
			}
		};

		struct KeyHasher {
			uint64_t operator()(const Key& key) const;
		};

		vk::PipelineRasterizationStateCreateInfo m_rasterizer;
		vk::PipelineDepthStencilStateCreateInfo m_depth_stencil;
		vk::PipelineColorBlendAttachmentState m_color_blend_attachment[4];
		vk::PipelineColorBlendStateCreateInfo m_color_blending;
		Vector<vk::PipelineShaderStageCreateInfo> m_stages;

		FlatMap<Identifier, vk::Pipeline> m_pipelines;

	private:
		vk::Pipeline find_or_create_pipeline(VulkanStateManager* manager);

	public:
		VulkanMeshPipeline(const RHIMeshPipelineInitializer* pipeline);
		VulkanMeshPipeline& flush(VulkanContext* ctx) override;
		~VulkanMeshPipeline();
	};

	class VulkanComputePipeline : public VulkanPipeline
	{
	private:
		vk::Pipeline m_pipeline;

	public:
		VulkanComputePipeline(const RHIComputePipelineInitializer* pipeline);
		VulkanPipeline& flush(VulkanContext* ctx) override;
		~VulkanComputePipeline();
	};

	class VulkanRayTracingPipeline : public VulkanPipeline
	{
	private:
		VulkanBuffer* m_sbt;
		size_t m_groups;
		vk::Pipeline m_pipeline;

	public:
		VulkanRayTracingPipeline(const RHIRayTracingPipelineInitializer* pipeline);
		VulkanRayTracingPipeline& flush(VulkanContext* ctx) override;
		~VulkanRayTracingPipeline();

		inline size_t groups() const { return m_groups; }
		inline VulkanBuffer* shader_binding_table() const { return m_sbt; }
	};
}// namespace Engine
