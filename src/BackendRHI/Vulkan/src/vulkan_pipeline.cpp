#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <RHI/initializers.hpp>
#include <vulkan_api.hpp>
#include <vulkan_bindless.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>

namespace Engine
{
	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const RHIGraphicsPipelineInitializer* pipeline)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);

		if (pipeline->vertex_shader)
			stages |= vk::ShaderStageFlagBits::eVertex;

		if (pipeline->tessellation_control_shader)
			stages |= vk::ShaderStageFlagBits::eTessellationControl;

		if (pipeline->tessellation_shader)
			stages |= vk::ShaderStageFlagBits::eTessellationEvaluation;

		if (pipeline->geometry_shader)
			stages |= vk::ShaderStageFlagBits::eGeometry;

		if (pipeline->fragment_shader)
			stages |= vk::ShaderStageFlagBits::eFragment;

		return stages;
	}

	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const RHIMeshPipelineInitializer* pipeline)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);

		if (pipeline->task_shader)
			stages |= vk::ShaderStageFlagBits::eTaskEXT;

		if (pipeline->mesh_shader)
			stages |= vk::ShaderStageFlagBits::eMeshEXT;

		if (pipeline->fragment_shader)
			stages |= vk::ShaderStageFlagBits::eFragment;

		return stages;
	}

	static FORCE_INLINE vk::ShaderStageFlagBits shader_stage_of(RHIRayTracingShaderGroupType type)
	{
		switch (type)
		{
			case RHIRayTracingShaderGroupType::GeneralMiss: return vk::ShaderStageFlagBits::eMissKHR;
			case RHIRayTracingShaderGroupType::GeneralCallable: return vk::ShaderStageFlagBits::eCallableKHR;
			default: return vk::ShaderStageFlagBits::eRaygenKHR;
		}
	}

	static inline vk::ShaderModule vulkan_shader_of(RHIShader* shader)
	{
		return shader->as<VulkanShader>()->module();
	}

	static FORCE_INLINE bool is_descriptor_dirty(VulkanStateManager* manager, const VulkanPipelineLayout::Descriptor& descriptor)
	{
		switch (descriptor.type)
		{
			case vk::DescriptorType::eSampledImage: return manager->srv_images.is_dirty(descriptor.binding);
			case vk::DescriptorType::eSampler: return manager->samplers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eCombinedImageSampler:
				return manager->srv_images.is_dirty(descriptor.binding) || manager->samplers.is_dirty(descriptor.binding);

			case vk::DescriptorType::eStorageImage: return manager->uav_images.is_dirty(descriptor.binding);
			case vk::DescriptorType::eUniformBufferDynamic: return manager->uniform_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageBuffer: return manager->storage_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eUniformTexelBuffer: return manager->uniform_texel_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageTexelBuffer: return manager->storage_texel_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eAccelerationStructureKHR:
				return manager->acceleration_structures.is_dirty(descriptor.binding);
			default: return false;
		}
	}

	static vk::PipelineDepthStencilStateCreateInfo create_depth_stencil(VulkanStateManager* manager)
	{
		static auto stencil_op_state = [](const RHIStencilState& pipeline) {
			vk::StencilOpState out_state;
			out_state.setReference(pipeline.reference)
			        .setWriteMask(pipeline.write_mask)
			        .setCompareMask(pipeline.compare_mask)
			        .setCompareOp(VulkanEnums::compare_of(pipeline.compare))
			        .setFailOp(VulkanEnums::stencil_of(pipeline.fail))
			        .setPassOp(VulkanEnums::stencil_of(pipeline.depth_pass))
			        .setDepthFailOp(VulkanEnums::stencil_of(pipeline.depth_fail));
			return out_state;
		};

		const RHIDepthState& depth     = manager->depth_state();
		const RHIStencilState& stencil = manager->stencil_state();
		vk::StencilOpState stencil_op  = stencil_op_state(stencil);

		vk::PipelineDepthStencilStateCreateInfo depth_stencil;
		depth_stencil.setDepthTestEnable(depth.enable)
		        .setDepthWriteEnable(depth.write_enable)
		        .setDepthBoundsTestEnable(vk::False)
		        .setDepthCompareOp(VulkanEnums::compare_of(depth.func))
		        .setMinDepthBounds(0.f)
		        .setMaxDepthBounds(0.f)
		        .setStencilTestEnable(stencil.enable)
		        .setFront(stencil_op)
		        .setBack(stencil_op);

		return depth_stencil;
	}

	static vk::PipelineColorBlendStateCreateInfo create_color_blend(VulkanStateManager* manager,
	                                                                vk::PipelineColorBlendAttachmentState* attachments)
	{
		const RHIBlendingState& blending = manager->blending_state();
		auto write_mask                  = VulkanEnums::color_component_flags_of(manager->write_mask());

		{
			auto& attachment = attachments[0];

			attachment.setBlendEnable(blending.enable)
			        .setSrcColorBlendFactor(VulkanEnums::blend_func_of(blending.src_color_func, false))
			        .setDstColorBlendFactor(VulkanEnums::blend_func_of(blending.dst_color_func, false))
			        .setColorBlendOp(VulkanEnums::blend_of(blending.color_op))
			        .setSrcAlphaBlendFactor(VulkanEnums::blend_func_of(blending.src_alpha_func, true))
			        .setDstAlphaBlendFactor(VulkanEnums::blend_func_of(blending.dst_alpha_func, true))
			        .setAlphaBlendOp(VulkanEnums::blend_of(blending.alpha_op))
			        .setColorWriteMask(write_mask);
		}

		for (int i = 1; i < 4; ++i)
		{
			attachments[i] = attachments[0];
		}

		vk::PipelineColorBlendStateCreateInfo info;
		return info.setAttachmentCount(4).setPAttachments(attachments).setLogicOpEnable(false);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		m_layout->release();
	}

	bool VulkanPipeline::is_dirty_state(VulkanStateManager* manager) const
	{
		trinex_profile_cpu_n("VulkanPipeline::is_dirty_state");
		size_t descriptors_count = m_layout->descriptors_count();

		if (manager->is_dirty(VulkanStateManager::Pipeline))
			return descriptors_count != 0;

		for (size_t i = 0; i < descriptors_count; ++i)
		{
			if (is_descriptor_dirty(manager, m_layout->descriptor(i)))
				return true;
		}

		return false;
	}

	VulkanPipelineLayout* VulkanPipeline::create_layout(const RHIShaderParameterInfo* parameter, size_t count,
	                                                    vk::ShaderStageFlags stages)
	{
		if (m_layout)
			m_layout->release();

		m_layout = API->pipeline_layout_manager()->allocate(parameter, count, stages);
		return m_layout;
	}

	VulkanPipeline& VulkanPipeline::flush_descriptors(VulkanContext* ctx, vk::PipelineBindPoint bind_point)
	{
		trinex_profile_cpu_n("VulkanPipeline::flush_descriptors");
		VulkanStateManager* state = ctx->state();
		if (!is_dirty_state(state))
			return *this;

		vk::DescriptorSet set                = VulkanDescriptorSetAllocator::instance()->allocate(m_layout, state);
		vk::DescriptorSet descriptor_sets[2] = {set, API->descriptor_heap()->descriptor_set()};

		StackByteAllocator::Mark mark;
		size_t uniforms_count   = m_layout->uniform_buffers_count();
		const auto* descriptors = m_layout->descriptors();
		uint32_t* offsets       = StackAllocator<uint32_t>::allocate(uniforms_count);

		for (uint64_t i = 0; i < uniforms_count; ++i)
		{
			offsets[i] = state->uniform_buffers.resource(descriptors[i].binding).offset;
		}

		ctx->handle()->bindDescriptorSets(bind_point, m_layout->layout(), 0, descriptor_sets,
		                                  vk::ArrayProxy<uint32_t>(uniforms_count, offsets));
		return *this;
	}

	vk::Pipeline VulkanGraphicsPipeline::find_or_create_pipeline(VulkanStateManager* manager)
	{
		auto rt = manager->render_target();

		auto link_id   = manager->graphics_pipeline_id(m_vertex_attributes, m_vertex_attributes_count);
		auto& pipeline = m_pipelines[link_id];

		if (pipeline)
		{
			return pipeline;
		}

		static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));
		static vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

		StackByteAllocator::Mark mark;

		m_input_assembly.setTopology(VulkanEnums::primitive_topology_of(manager->primitive_topology()));
		m_rasterizer.setPolygonMode(VulkanEnums::polygon_mode_of(manager->polygon_mode()));
		m_rasterizer.setCullMode(VulkanEnums::cull_mode_of(manager->cull_mode()));
		m_rasterizer.setFrontFace(VulkanEnums::face_of(manager->front_face()));

		vk::PipelineColorBlendAttachmentState attachments[4];
		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);
		vk::PipelineMultisampleStateCreateInfo multisampling;
		vk::PipelineVertexInputStateCreateInfo vertex_input =
		        manager->create_vertex_input(m_vertex_attributes, m_vertex_attributes_count);
		vk::PipelineDepthStencilStateCreateInfo depth_stencil = create_depth_stencil(manager);
		vk::PipelineColorBlendStateCreateInfo color_blend     = create_color_blend(manager, attachments);

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, &vertex_input, &m_input_assembly, nullptr, &viewport_state,
		                                             &m_rasterizer, &multisampling, &depth_stencil, &color_blend, &dynamic_state,
		                                             layout()->layout(), rt->m_render_pass->render_pass(), 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const RHIGraphicsPipelineInitializer* pipeline)
	{
		create_layout(pipeline->parameters, pipeline->parameters_count, parse_stages_flags(pipeline));

		m_input_assembly.primitiveRestartEnable = vk::False;
		m_input_assembly.topology               = vk::PrimitiveTopology::eTriangleList;
		m_rasterizer.polygonMode                = vk::PolygonMode::eFill;
		m_rasterizer.cullMode                   = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace                  = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth                  = 1.f;

		m_vertex_attributes_count = pipeline->vertex_attributes_count;
		if (m_vertex_attributes_count)
			m_vertex_attributes = Allocator<VulkanVertexAttribute>::allocate(m_vertex_attributes_count);

		for (size_t index = 0; index < pipeline->vertex_attributes_count; ++index)
		{
			auto& src = pipeline->vertex_attributes[index];
			auto& dst = m_vertex_attributes[index];

			dst.semantic = src.semantic;
			dst.binding  = src.binding;
			dst.format   = VulkanEnums::vertex_format_of(src.format);
		}

		static vk::ShaderStageFlagBits graphics_stages[] = {
		        vk::ShaderStageFlagBits::eVertex,
		        vk::ShaderStageFlagBits::eTessellationControl,
		        vk::ShaderStageFlagBits::eTessellationEvaluation,
		        vk::ShaderStageFlagBits::eGeometry,
		        vk::ShaderStageFlagBits::eFragment,
		};

		for (uint_t i = 0; i < 5; ++i)
		{
			if (pipeline->shaders[i])
			{
				m_stages.emplace_back(vk::PipelineShaderStageCreateFlags(), graphics_stages[i],
				                      vulkan_shader_of(pipeline->shaders[i]), "main");
			}
		}
	}

	bool VulkanGraphicsPipeline::is_dirty_vertex_input(VulkanStateManager* manager)
	{
		for (uint16_t i = 0; i < m_vertex_attributes_count; ++i)
		{
			const VulkanVertexAttribute& attribute = m_vertex_attributes[i];

			if (manager->vertex_attributes.is_dirty(attribute.semantic))
				return true;

			auto stream = manager->vertex_attributes.resource(attribute.semantic).stream;

			if (manager->vertex_streams.is_dirty(stream))
				return true;
		}

		return false;
	}

	VulkanPipeline& VulkanGraphicsPipeline::flush(VulkanContext* ctx)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");

		if (ctx->state()->is_dirty(VulkanStateManager::GraphicsMask) || is_dirty_vertex_input(ctx->state()))
		{
			auto cmd              = ctx->handle();
			auto current_pipeline = find_or_create_pipeline(ctx->state());
			cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
		}

		VulkanPipeline::flush_descriptors(ctx, vk::PipelineBindPoint::eGraphics);
		return *this;
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		for (auto [id, pipeline] : m_pipelines)
		{
			DESTROY_CALL(destroyPipeline, pipeline);
		}

		if (m_vertex_attributes)
			Allocator<VulkanVertexAttribute>::deallocate(m_vertex_attributes);
	}

	uint64_t VulkanMeshPipeline::KeyHasher::operator()(const Key& key) const
	{
		return memory_hash(&key, sizeof(key));
	}

	VulkanMeshPipeline::VulkanMeshPipeline(const RHIMeshPipelineInitializer* pipeline)
	{
		create_layout(pipeline->parameters, pipeline->parameters_count, parse_stages_flags(pipeline));

		m_rasterizer.polygonMode = vk::PolygonMode::eFill;
		m_rasterizer.cullMode    = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace   = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth   = 1.f;


		static vk::ShaderStageFlagBits graphics_stages[] = {
		        vk::ShaderStageFlagBits::eTaskEXT,
		        vk::ShaderStageFlagBits::eMeshEXT,
		        vk::ShaderStageFlagBits::eFragment,
		};

		for (uint_t i = 0; i < 5; ++i)
		{
			if (pipeline->shaders[i])
			{
				m_stages.emplace_back(vk::PipelineShaderStageCreateFlags(), graphics_stages[i],
				                      vulkan_shader_of(pipeline->shaders[i]), "main");
			}
		}
	}

	vk::Pipeline VulkanMeshPipeline::find_or_create_pipeline(VulkanStateManager* manager)
	{
		vk::Pipeline& pipeline = m_pipelines[manager->mesh_pipeline_id()];

		if (pipeline)
		{
			return pipeline;
		}

		static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));
		static vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

		m_rasterizer.setPolygonMode(VulkanEnums::polygon_mode_of(manager->polygon_mode()));
		m_rasterizer.setCullMode(VulkanEnums::cull_mode_of(manager->cull_mode()));
		m_rasterizer.setFrontFace(VulkanEnums::face_of(manager->front_face()));

		vk::PipelineColorBlendAttachmentState attachments[4];
		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);
		vk::PipelineMultisampleStateCreateInfo multisampling;
		vk::PipelineDepthStencilStateCreateInfo depth_stencil = create_depth_stencil(manager);
		vk::PipelineColorBlendStateCreateInfo color_blend     = create_color_blend(manager, attachments);

		auto rt = manager->render_target();

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, nullptr, nullptr, nullptr, &viewport_state, &m_rasterizer,
		                                             &multisampling, &depth_stencil, &color_blend, &dynamic_state,
		                                             layout()->layout(), rt->m_render_pass->render_pass(), 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanMeshPipeline& VulkanMeshPipeline::flush(VulkanContext* ctx)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");

		if (ctx->state()->is_dirty(VulkanStateManager::GraphicsMask))
		{
			auto cmd              = ctx->handle();
			auto current_pipeline = find_or_create_pipeline(ctx->state());
			cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
		}

		VulkanPipeline::flush_descriptors(ctx, vk::PipelineBindPoint::eGraphics);
		return *this;
	}

	VulkanMeshPipeline::~VulkanMeshPipeline()
	{
		for (auto [id, pipeline] : m_pipelines)
		{

			DESTROY_CALL(destroyPipeline, pipeline);
		}
	}

	VulkanComputePipeline::VulkanComputePipeline(const RHIComputePipelineInitializer* pipeline)
	{
		vk::PipelineShaderStageCreateInfo stage;
		if (pipeline->compute_shader)
		{
			stage.setStage(vk::ShaderStageFlagBits::eCompute)
			        .setModule(vulkan_shader_of(pipeline->compute_shader))
			        .setPName("main");
		}
		else
		{
			error_log("VulkanPipeline", "Cannot init pipeline, because 'Compute' shader is not valid");
		}

		create_layout(pipeline->parameters, pipeline->parameters_count, vk::ShaderStageFlagBits::eCompute);
		vk::ComputePipelineCreateInfo info({}, stage, layout()->layout());

		auto result = API->m_device.createComputePipeline({}, info);
		if (result.result != vk::Result::eSuccess)
		{
			error_log("VulkanComputePipeline", "Failed to create pipeline");
		}

		m_pipeline = result.value;
	}

	VulkanPipeline& VulkanComputePipeline::flush(VulkanContext* ctx)
	{
		if (ctx->state()->is_dirty(VulkanStateManager::Pipeline))
		{
			ctx->handle()->bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
		}

		VulkanPipeline::flush_descriptors(ctx, vk::PipelineBindPoint::eCompute);
		return *this;
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		DESTROY_CALL(destroyPipeline, m_pipeline);
	}

	static void align_shader_binding_table(byte* storage, size_t count, size_t handle, size_t aligned_handle)
	{
		if (handle == aligned_handle)
			return;

		for (size_t i = count; i-- > 0;)
		{
			byte* src = storage + i * handle;
			byte* dst = storage + i * aligned_handle;
			std::memmove(dst, src, handle);

			if (aligned_handle > handle)
				std::memset(dst + handle, 0, aligned_handle - handle);
		}
	}

	VulkanRayTracingPipeline::VulkanRayTracingPipeline(const RHIRayTracingPipelineInitializer* pipeline)
	    : m_groups(pipeline->groups_count)
	{
		StackByteAllocator::Mark mark;

		struct {
			vk::RayTracingPipelineCreateInfoKHR pipeline_info{};
			vk::ShaderStageFlags stages;
			vk::RayTracingShaderGroupCreateInfoKHR* groups_info;
			vk::PipelineShaderStageCreateInfo* stages_info;
		} state;

		state.groups_info = StackAllocator<vk::RayTracingShaderGroupCreateInfoKHR>::allocate(pipeline->groups_count);
		state.stages_info = StackAllocator<vk::PipelineShaderStageCreateInfo>::allocate(pipeline->groups_count * 3);

		state.pipeline_info.pStages                      = state.stages_info;
		state.pipeline_info.groupCount                   = pipeline->groups_count;
		state.pipeline_info.pGroups                      = state.groups_info;
		state.pipeline_info.maxPipelineRayRecursionDepth = pipeline->max_recursion;

		auto find_shader_index = [&](const RHIShader* handle, vk::ShaderStageFlagBits stage) -> uint32_t {
			if (handle == nullptr)
				return vk::ShaderUnusedKhr;

			const VulkanShader* shader = static_cast<const VulkanShader*>(handle);
			auto module                = shader->module();

			state.stages |= stage;

			for (uint32_t i = 0, count = state.pipeline_info.stageCount; i < count; ++i)
			{
				if (state.stages_info->module == module)
					return i;
			}

			uint32_t index = state.pipeline_info.stageCount++;
			new (state.stages_info + index) vk::PipelineShaderStageCreateInfo({}, stage, module, "main");
			return index;
		};

		for (size_t i = 0; i < pipeline->groups_count; ++i)
		{
			auto& src = pipeline->groups[i];
			auto& dst = state.groups_info[i];

			new (&dst) vk::RayTracingShaderGroupCreateInfoKHR(vk::RayTracingShaderGroupTypeKHR::eGeneral, vk::ShaderUnusedKhr,
			                                                  vk::ShaderUnusedKhr, vk::ShaderUnusedKhr, vk::ShaderUnusedKhr);

			if (src.type == RHIRayTracingShaderGroupType::TrianglesHit)
			{
				dst.type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;

				dst.closestHitShader = find_shader_index(src.closest_hit, vk::ShaderStageFlagBits::eClosestHitKHR);
				dst.anyHitShader     = find_shader_index(src.any_hit, vk::ShaderStageFlagBits::eAnyHitKHR);
			}
			else if (src.type == RHIRayTracingShaderGroupType::ProceduralHit)
			{
				dst.type = vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup;

				dst.intersectionShader = find_shader_index(src.intersection, vk::ShaderStageFlagBits::eIntersectionKHR);
				dst.closestHitShader   = find_shader_index(src.closest_hit, vk::ShaderStageFlagBits::eClosestHitKHR);
				dst.anyHitShader       = find_shader_index(src.any_hit, vk::ShaderStageFlagBits::eAnyHitKHR);
			}
			else
			{
				dst.generalShader = find_shader_index(src.general, shader_stage_of(src.type));
			}
		}

		create_layout(pipeline->parameters, pipeline->parameters_count, state.stages);
		state.pipeline_info.layout = layout()->layout();

		m_pipeline = API->m_device.createRayTracingPipelineKHR({}, {}, state.pipeline_info, nullptr, API->pfn).value;

		mark.reset();

		// Creating shader binding table
		{
			auto& props                        = API->ray_trace_properties();
			const uint32_t handle_size         = props.shaderGroupHandleSize;
			const uint32_t handle_size_aligned = align_up(handle_size, props.shaderGroupBaseAlignment);

			const size_t storage_size = pipeline->groups_count * handle_size_aligned;
			byte* storage             = StackByteAllocator::allocate(pipeline->groups_count * handle_size);

			auto result = API->m_device.getRayTracingShaderGroupHandlesKHR(m_pipeline, 0, pipeline->groups_count, storage_size,
			                                                               storage, API->pfn);
			if (result != vk::Result::eSuccess)
				throw EngineException("Failed to create shader binding table!");

			align_shader_binding_table(storage, pipeline->groups_count, handle_size, handle_size_aligned);

			m_sbt = trx_new VulkanBuffer();
			//m_sbt->create(storage_size, storage, RHIBufferCreateFlags::ShaderBindingTable | RHIBufferCreateFlags::DeviceAddress);
		}
	}

	VulkanRayTracingPipeline& VulkanRayTracingPipeline::flush(VulkanContext* ctx)
	{
		if (ctx->state()->is_dirty(VulkanStateManager::Pipeline))
		{
			ctx->handle()->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipeline);
		}

		VulkanPipeline::flush_descriptors(ctx, vk::PipelineBindPoint::eRayTracingKHR);
		return *this;
	}

	VulkanRayTracingPipeline::~VulkanRayTracingPipeline()
	{
		DESTROY_CALL(destroyPipeline, m_pipeline);
		trx_delete_inline(m_sbt);
	}

	VulkanContext& VulkanContext::bind_pipeline(RHIPipeline* pipeline)
	{
		m_state_manager->bind(static_cast<VulkanPipeline*>(pipeline));
		return *this;
	}

	RHIPipeline* VulkanAPI::create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)
	{
		return trx_new VulkanGraphicsPipeline(pipeline);
	}

	RHIPipeline* VulkanAPI::create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline)
	{
		return trx_new VulkanMeshPipeline(pipeline);
	}

	RHIPipeline* VulkanAPI::create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)
	{
		return trx_new VulkanComputePipeline(pipeline);
	}

	RHIPipeline* VulkanAPI::create_ray_tracing_pipeline(const RHIRayTracingPipelineInitializer* pipeline)
	{
		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)))
			return trx_new VulkanRayTracingPipeline(pipeline);
		return nullptr;
	}
}// namespace Engine
