#include <Core/etl/templates.hpp>
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
#include <vulkan_resource_view.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_texture.hpp>

namespace Trinex
{
	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const RHIGraphicsPipelineDesc& desc)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);

		if (desc.vertex_shader)
			stages |= vk::ShaderStageFlagBits::eVertex;

		if (desc.tessellation_control_shader)
			stages |= vk::ShaderStageFlagBits::eTessellationControl;

		if (desc.tessellation_shader)
			stages |= vk::ShaderStageFlagBits::eTessellationEvaluation;

		if (desc.geometry_shader)
			stages |= vk::ShaderStageFlagBits::eGeometry;

		if (desc.fragment_shader)
			stages |= vk::ShaderStageFlagBits::eFragment;

		return stages;
	}

	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const RHIMeshPipelineDesc& desc)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);

		if (desc.task_shader)
			stages |= vk::ShaderStageFlagBits::eTaskEXT;

		if (desc.mesh_shader)
			stages |= vk::ShaderStageFlagBits::eMeshEXT;

		if (desc.fragment_shader)
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

	static FORCE_INLINE bool is_descriptor_dirty(VulkanContext* context, const VulkanPipelineLayout::Descriptor& descriptor)
	{
		switch (descriptor.type)
		{
			case vk::DescriptorType::eSampledImage: return context->srv_images.is_dirty(descriptor.binding);
			case vk::DescriptorType::eSampler: return context->samplers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eCombinedImageSampler:
				return context->srv_images.is_dirty(descriptor.binding) || context->samplers.is_dirty(descriptor.binding);

			case vk::DescriptorType::eStorageImage: return context->uav_images.is_dirty(descriptor.binding);
			case vk::DescriptorType::eUniformBufferDynamic: return context->uniform_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageBuffer: return context->storage_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eUniformTexelBuffer: return context->uniform_texel_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageTexelBuffer: return context->storage_texel_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eAccelerationStructureKHR:
				return context->acceleration_structures.is_dirty(descriptor.binding);
			default: return false;
		}
	}

	static vk::PipelineDepthStencilStateCreateInfo create_depth_stencil(VulkanContext* context)
	{
		static auto stencil_op_state = [](const RHIDepthStencilState::Stencil& pipeline) {
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

		const RHIDepthStencilState& state = context->depth_stencil_state();
		vk::StencilOpState stencil_op     = stencil_op_state(state.stencil);

		vk::PipelineDepthStencilStateCreateInfo depth_stencil;
		depth_stencil.setDepthTestEnable(state.depth.is_enabled())
		        .setDepthWriteEnable(state.depth.write)
		        .setDepthBoundsTestEnable(vk::False)
		        .setDepthCompareOp(VulkanEnums::compare_of(state.depth.func))
		        .setMinDepthBounds(0.f)
		        .setMaxDepthBounds(0.f)
		        .setStencilTestEnable(state.stencil.is_enabled())
		        .setFront(stencil_op)
		        .setBack(stencil_op);

		return depth_stencil;
	}

	static vk::PipelineColorBlendStateCreateInfo create_color_blend(VulkanContext* context,
	                                                                vk::PipelineColorBlendAttachmentState* attachments)
	{
		const RHIBlendingState& blending = context->blending_state();
		auto write_mask                  = VulkanEnums::color_component_flags_of(context->write_mask());

		{
			auto& attachment = attachments[0];

			attachment.setBlendEnable(blending.is_enabled())
			        .setSrcColorBlendFactor(VulkanEnums::blend_func_of(blending.src_color_func))
			        .setDstColorBlendFactor(VulkanEnums::blend_func_of(blending.dst_color_func))
			        .setColorBlendOp(VulkanEnums::blend_of(blending.color_op))
			        .setSrcAlphaBlendFactor(VulkanEnums::blend_func_of(blending.src_alpha_func))
			        .setDstAlphaBlendFactor(VulkanEnums::blend_func_of(blending.dst_alpha_func))
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

	bool VulkanPipeline::is_dirty_state(VulkanContext* context) const
	{
		trinex_profile_cpu_n("VulkanPipeline::is_dirty_state");
		usize descriptors_count = m_layout->descriptors_count();

		if (context->is_dirty(VulkanContext::Pipeline))
			return descriptors_count != 0;

		for (usize i = 0; i < descriptors_count; ++i)
		{
			if (is_descriptor_dirty(context, m_layout->descriptor(i)))
				return true;
		}

		return false;
	}

	VulkanPipelineLayout* VulkanPipeline::create_layout(const RHIShaderParameterInfo* parameter, usize count,
	                                                    vk::ShaderStageFlags stages)
	{
		if (m_layout)
			m_layout->release();

		m_layout = API->pipeline_layout_manager()->allocate(parameter, count, stages);
		return m_layout;
	}

	VulkanPipeline& VulkanPipeline::flush_descriptors(VulkanContext* context, vk::PipelineBindPoint bind_point)
	{
		trinex_profile_cpu_n("VulkanPipeline::flush_descriptors");

		if (!is_dirty_state(context))
			return *this;

		vk::DescriptorSet sets[2] = {
		        VulkanDescriptorSetAllocator::instance()->allocate(m_layout, context),
		        API->descriptor_heap()->descriptor_set(),
		};

		StackByteAllocator::Mark mark;
		usize uniforms_count    = m_layout->uniform_buffers_count();
		const auto* descriptors = m_layout->descriptors();
		u32* offsets            = StackAllocator<u32>::allocate(uniforms_count);

		for (u64 i = 0; i < uniforms_count; ++i)
		{
			offsets[i] = context->uniform_buffers.resource(descriptors[i].binding).offset;
		}

		{
			trinex_profile_cpu_n("bindDescriptorSets");
			context->handle()->bindDescriptorSets(bind_point, m_layout->layout(), 0, sets,
			                                      vk::ArrayProxy<u32>(uniforms_count, offsets));
		}
		return *this;
	}

	vk::Pipeline VulkanGraphicsPipeline::find_or_create_pipeline(VulkanContext* context)
	{
		auto link_id   = context->graphics_pipeline_id(m_vertex_attributes, m_vertex_attributes_count);
		auto& pipeline = m_pipelines[link_id];

		if (pipeline)
		{
			return pipeline;
		}

		StackByteAllocator::Mark mark;

		m_input_assembly.setTopology(VulkanEnums::primitive_topology_of(context->primitive_topology()));
		m_rasterizer.setPolygonMode(VulkanEnums::polygon_mode_of(context->polygon_mode()));
		m_rasterizer.setCullMode(VulkanEnums::cull_mode_of(context->cull_mode()));
		m_rasterizer.setFrontFace(VulkanEnums::face_of(context->front_face()));

		vk::PipelineColorBlendAttachmentState attachments[4];

		vk::DynamicState dynamic_states[10];
		usize dynamic_states_count = 0;

		dynamic_states[dynamic_states_count++] = vk::DynamicState::eViewport;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eScissor;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eDepthBias;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eDepthBounds;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eStencilCompareMask;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eStencilWriteMask;
		dynamic_states[dynamic_states_count++] = vk::DynamicState::eStencilReference;

		vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));

		vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states_count, dynamic_states);
		vk::PipelineMultisampleStateCreateInfo multisampling;
		vk::PipelineVertexInputStateCreateInfo vertex_input =
		        context->create_vertex_input(m_vertex_attributes, m_vertex_attributes_count);
		vk::PipelineDepthStencilStateCreateInfo depth_stencil = create_depth_stencil(context);
		vk::PipelineColorBlendStateCreateInfo color_blend     = create_color_blend(context, attachments);
		vk::PipelineRenderingCreateInfo rendering_info        = context->framebuffer().pipeline_create_info();

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, &vertex_input, &m_input_assembly, nullptr, &viewport_state,
		                                             &m_rasterizer, &multisampling, &depth_stencil, &color_blend, &dynamic_state,
		                                             layout()->layout(), {}, 0, {}, {}, &rendering_info);

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);
		trinex_assert(pipeline_result.result == vk::Result::eSuccess);

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const RHIGraphicsPipelineDesc& desc)
	{
		create_layout(desc.parameters, desc.parameters_count, parse_stages_flags(desc));

		m_input_assembly.primitiveRestartEnable = vk::False;
		m_input_assembly.topology               = vk::PrimitiveTopology::eTriangleList;
		m_rasterizer.polygonMode                = vk::PolygonMode::eFill;
		m_rasterizer.cullMode                   = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace                  = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth                  = 1.f;

		m_vertex_attributes_count = desc.vertex_attributes_count;
		if (m_vertex_attributes_count)
			m_vertex_attributes = Allocator<VulkanVertexAttribute>::allocate(m_vertex_attributes_count);

		for (usize index = 0; index < desc.vertex_attributes_count; ++index)
		{
			auto& src = desc.vertex_attributes[index];
			auto& dst = m_vertex_attributes[index];

			dst.semantic = src.semantic;
			dst.binding  = src.binding;
		}

		static vk::ShaderStageFlagBits graphics_stages[] = {
		        vk::ShaderStageFlagBits::eVertex,
		        vk::ShaderStageFlagBits::eTessellationControl,
		        vk::ShaderStageFlagBits::eTessellationEvaluation,
		        vk::ShaderStageFlagBits::eGeometry,
		        vk::ShaderStageFlagBits::eFragment,
		};

		for (u32 i = 0; i < 5; ++i)
		{
			if (desc.shaders[i])
			{
				vk::PipelineShaderStageCreateFlags flags = {};
				m_stages.emplace_back(flags, graphics_stages[i], vulkan_shader_of(desc.shaders[i]), "main");
			}
		}
	}

	bool VulkanGraphicsPipeline::is_dirty_vertex_input(VulkanContext* context)
	{
		for (u16 i = 0; i < m_vertex_attributes_count; ++i)
		{
			const VulkanVertexAttribute& attribute = m_vertex_attributes[i];

			if (context->vertex_attributes.is_dirty(attribute.semantic))
				return true;

			auto stream = context->vertex_attributes.resource(attribute.semantic).stream;

			if (context->vertex_streams.is_dirty(stream))
				return true;
		}

		return false;
	}

	VulkanPipeline& VulkanGraphicsPipeline::flush(VulkanContext* ctx)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");

		if (ctx->is_dirty(VulkanContext::GraphicsMask) || is_dirty_vertex_input(ctx))
		{
			auto cmd              = ctx->handle();
			auto current_pipeline = find_or_create_pipeline(ctx);
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

	u64 VulkanMeshPipeline::KeyHasher::operator()(const Key& key) const
	{
		return memory_hash(&key, sizeof(key));
	}

	VulkanMeshPipeline::VulkanMeshPipeline(const RHIMeshPipelineDesc& desc)
	{
		create_layout(desc.parameters, desc.parameters_count, parse_stages_flags(desc));

		m_rasterizer.polygonMode = vk::PolygonMode::eFill;
		m_rasterizer.cullMode    = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace   = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth   = 1.f;


		static vk::ShaderStageFlagBits graphics_stages[] = {
		        vk::ShaderStageFlagBits::eTaskEXT,
		        vk::ShaderStageFlagBits::eMeshEXT,
		        vk::ShaderStageFlagBits::eFragment,
		};

		for (u32 i = 0; i < 5; ++i)
		{
			if (desc.shaders[i])
			{
				vk::PipelineShaderStageCreateFlags flags = {};
				m_stages.emplace_back(flags, graphics_stages[i], vulkan_shader_of(desc.shaders[i]), "main");
			}
		}
	}

	vk::Pipeline VulkanMeshPipeline::find_or_create_pipeline(VulkanContext* context)
	{
		vk::Pipeline& pipeline = m_pipelines[context->pipeline_state_id()];

		if (pipeline)
		{
			return pipeline;
		}


		m_rasterizer.setPolygonMode(VulkanEnums::polygon_mode_of(context->polygon_mode()));
		m_rasterizer.setCullMode(VulkanEnums::cull_mode_of(context->cull_mode()));
		m_rasterizer.setFrontFace(VulkanEnums::face_of(context->front_face()));

		vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));

		vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
		vk::PipelineColorBlendAttachmentState attachments[4];
		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);
		vk::PipelineMultisampleStateCreateInfo multisampling;
		vk::PipelineDepthStencilStateCreateInfo depth_stencil = create_depth_stencil(context);
		vk::PipelineColorBlendStateCreateInfo color_blend     = create_color_blend(context, attachments);
		vk::PipelineRenderingCreateInfo rendering_info        = context->framebuffer().pipeline_create_info();

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, nullptr, nullptr, nullptr, &viewport_state, &m_rasterizer,
		                                             &multisampling, &depth_stencil, &color_blend, &dynamic_state,
		                                             layout()->layout(), {}, 0, {}, {}, &rendering_info);

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);
		trinex_assert(pipeline_result.result == vk::Result::eSuccess);

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanMeshPipeline& VulkanMeshPipeline::flush(VulkanContext* ctx)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");

		if (ctx->is_dirty(VulkanContext::GraphicsMask))
		{
			auto cmd              = ctx->handle();
			auto current_pipeline = find_or_create_pipeline(ctx);
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

	VulkanComputePipeline::VulkanComputePipeline(const RHIComputePipelineDesc& desc)
	{
		vk::PipelineShaderStageCreateInfo stage;
		if (desc.compute_shader)
		{
			stage.setStage(vk::ShaderStageFlagBits::eCompute).setModule(vulkan_shader_of(desc.compute_shader)).setPName("main");
		}
		else
		{
			error_log("VulkanPipeline", "Cannot init pipeline, because 'Compute' shader is not valid");
		}

		create_layout(desc.parameters, desc.parameters_count, vk::ShaderStageFlagBits::eCompute);
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
		if (ctx->is_dirty(VulkanContext::Pipeline))
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

	static void align_shader_binding_table(u8* storage, usize count, usize handle, usize aligned_handle)
	{
		if (handle == aligned_handle)
			return;

		for (usize i = count; i-- > 0;)
		{
			u8* src = storage + i * handle;
			u8* dst = storage + i * aligned_handle;
			std::memmove(dst, src, handle);

			if (aligned_handle > handle)
				std::memset(dst + handle, 0, aligned_handle - handle);
		}
	}

	VulkanRayTracingPipeline::VulkanRayTracingPipeline(const RHIRayTracingPipelineDesc& desc) : m_groups(desc.groups_count)
	{
		StackByteAllocator::Mark mark;

		struct {
			vk::RayTracingPipelineCreateInfoKHR pipeline_info{};
			vk::ShaderStageFlags stages;
			vk::RayTracingShaderGroupCreateInfoKHR* groups_info;
			vk::PipelineShaderStageCreateInfo* stages_info;
		} state;

		state.groups_info = StackAllocator<vk::RayTracingShaderGroupCreateInfoKHR>::allocate(desc.groups_count);
		state.stages_info = StackAllocator<vk::PipelineShaderStageCreateInfo>::allocate(desc.groups_count * 3);

		state.pipeline_info.pStages                      = state.stages_info;
		state.pipeline_info.groupCount                   = desc.groups_count;
		state.pipeline_info.pGroups                      = state.groups_info;
		state.pipeline_info.maxPipelineRayRecursionDepth = desc.max_recursion;

		auto find_shader_index = [&](const RHIShader* handle, vk::ShaderStageFlagBits stage) -> u32 {
			if (handle == nullptr)
				return vk::ShaderUnusedKHR;

			const VulkanShader* shader = static_cast<const VulkanShader*>(handle);
			auto module                = shader->module();

			state.stages |= stage;

			for (u32 i = 0, count = state.pipeline_info.stageCount; i < count; ++i)
			{
				if (state.stages_info->module == module)
					return i;
			}

			u32 index = state.pipeline_info.stageCount++;
			new (state.stages_info + index) vk::PipelineShaderStageCreateInfo({}, stage, module, "main");
			return index;
		};

		for (usize i = 0; i < desc.groups_count; ++i)
		{
			auto& src = desc.groups[i];
			auto& dst = state.groups_info[i];

			new (&dst) vk::RayTracingShaderGroupCreateInfoKHR(vk::RayTracingShaderGroupTypeKHR::eGeneral, vk::ShaderUnusedKHR,
			                                                  vk::ShaderUnusedKHR, vk::ShaderUnusedKHR, vk::ShaderUnusedKHR);

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

		create_layout(desc.parameters, desc.parameters_count, state.stages);
		state.pipeline_info.layout = layout()->layout();

		m_pipeline = API->m_device.createRayTracingPipelineKHR({}, {}, state.pipeline_info, nullptr).value;

		mark.reset();

		// Creating shader binding table
		{
			auto& props                   = API->ray_trace_properties();
			const u32 handle_size         = props.shaderGroupHandleSize;
			const u32 handle_size_aligned = align_up(handle_size, props.shaderGroupBaseAlignment);

			const usize storage_size = desc.groups_count * handle_size_aligned;
			u8* storage              = StackByteAllocator::allocate(desc.groups_count * handle_size);

			auto result =
			        API->m_device.getRayTracingShaderGroupHandlesKHR(m_pipeline, 0, desc.groups_count, storage_size, storage);
			trinex_assert_msg(result == vk::Result::eSuccess, "Failed to create shader binding table!");

			align_shader_binding_table(storage, desc.groups_count, handle_size, handle_size_aligned);

			m_sbt = trx_new VulkanBuffer();
			//m_sbt->create(storage_size, storage, RHIBufferCreateFlags::ShaderBindingTable | RHIBufferCreateFlags::DeviceAddress);
		}
	}

	VulkanRayTracingPipeline& VulkanRayTracingPipeline::flush(VulkanContext* ctx)
	{
		if (ctx->is_dirty(VulkanContext::Pipeline))
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
		if (m_pipeline != pipeline)
		{
			m_pipeline = static_cast<VulkanPipeline*>(pipeline);
			m_dirty_flags |= Pipeline;
		}
		return *this;
	}

	RHIPipeline* VulkanAPI::create_graphics_pipeline(const RHIGraphicsPipelineDesc& desc)
	{
		return trx_new VulkanGraphicsPipeline(desc);
	}

	RHIPipeline* VulkanAPI::create_mesh_pipeline(const RHIMeshPipelineDesc& desc)
	{
		return trx_new VulkanMeshPipeline(desc);
	}

	RHIPipeline* VulkanAPI::create_compute_pipeline(const RHIComputePipelineDesc& desc)
	{
		return trx_new VulkanComputePipeline(desc);
	}

	RHIPipeline* VulkanAPI::create_ray_tracing_pipeline(const RHIRayTracingPipelineDesc& desc)
	{
		if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)))
			return trx_new VulkanRayTracingPipeline(desc);
		return nullptr;
	}
}// namespace Trinex
