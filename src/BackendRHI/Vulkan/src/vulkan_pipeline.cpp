#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/profiler.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <RHI/initializers.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
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

	static inline vk::ShaderModule vulkan_shader_of(RHIShader* shader)
	{
		return shader->as<VulkanShader>()->module();
	}

	static FORCE_INLINE void write_sampled_image(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		VulkanTextureSRV* srv  = manager->srv_images.resource(index);
		VulkanTexture* texture = srv->texture();

		texture->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::DescriptorImageInfo image_info({}, srv->view(), srv->texture()->layout());
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eSampledImage, image_info, {}, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_sampler(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		vk::Sampler sampler = manager->samplers.resource(index);

		vk::DescriptorImageInfo image_info(sampler, {}, vk::ImageLayout::eUndefined);
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eSampler, image_info, {}, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_combined_image_sampler(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		VulkanTextureSRV* srv  = manager->srv_images.resource(index);
		vk::Sampler sampler    = manager->samplers.resource(index);
		VulkanTexture* texture = srv->texture();

		texture->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::DescriptorImageInfo image_info(sampler, srv->view(), texture->layout());
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eCombinedImageSampler, image_info, {}, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_storage_image(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		VulkanTextureUAV* uav  = manager->uav_images.resource(index);
		VulkanTexture* texture = uav->texture();

		texture->change_layout(vk::ImageLayout::eGeneral);
		vk::DescriptorImageInfo image_info({}, uav->view(), uav->texture()->layout());
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eStorageImage, image_info, {}, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_uniform_buffer(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		auto buffer = manager->uniform_buffers.resource(index);
		vk::DescriptorBufferInfo buffer_info(buffer.buffer, buffer.offset, buffer.size);
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eUniformBuffer, {}, buffer_info, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_storage_buffer(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		auto buffer = manager->storage_buffers.resource(index);
		vk::DescriptorBufferInfo buffer_info(buffer.buffer, buffer.offset, buffer.size);
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eStorageBuffer, {}, buffer_info, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_uniform_texel_buffer(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		auto buffer = manager->uniform_texel_buffers.resource(index);
		vk::DescriptorBufferInfo buffer_info(buffer.buffer, buffer.offset, buffer.size);
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eUniformTexelBuffer, {}, buffer_info, {});
		API->m_device.updateDescriptorSets(write, {});
	}

	static FORCE_INLINE void write_storage_texel_buffer(vk::DescriptorSet& set, VulkanStateManager* manager, byte index)
	{
		auto buffer = manager->storage_texel_buffers.resource(index);
		vk::DescriptorBufferInfo buffer_info(buffer.buffer, buffer.offset, buffer.size);
		vk::WriteDescriptorSet write(set, index, 0, vk::DescriptorType::eStorageTexelBuffer, {}, buffer_info, {});
		API->m_device.updateDescriptorSets(write, {});
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
			case vk::DescriptorType::eUniformBuffer: return manager->uniform_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageBuffer: return manager->storage_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eUniformTexelBuffer: return manager->uniform_texel_buffers.is_dirty(descriptor.binding);
			case vk::DescriptorType::eStorageTexelBuffer: return manager->storage_texel_buffers.is_dirty(descriptor.binding);
			default: return false;
		}
	}

	VulkanPipeline::VulkanPipeline(const RHIShaderParameterInfo* parameter, size_t count, vk::ShaderStageFlags stages)
	{
		m_layout = API->create_pipeline_layout(parameter, count, stages);
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

	VulkanPipeline& VulkanPipeline::flush_descriptors(VulkanStateManager* manager, vk::PipelineBindPoint bind_point)
	{
		trinex_profile_cpu_n("VulkanPipeline::flush_descriptors");
		if (!is_dirty_state(manager))
			return *this;

		vk::DescriptorSet set = API->descriptor_set_allocator()->allocate(m_layout);

		size_t descriptors_count = m_layout->descriptors_count();

		for (size_t i = 0; i < descriptors_count; ++i)
		{
			auto descriptor = m_layout->descriptor(i);
			byte binding    = descriptor.binding;

			switch (descriptor.type)
			{
				case vk::DescriptorType::eSampledImage: write_sampled_image(set, manager, binding); break;
				case vk::DescriptorType::eSampler: write_sampler(set, manager, binding); break;
				case vk::DescriptorType::eCombinedImageSampler: write_combined_image_sampler(set, manager, binding); break;
				case vk::DescriptorType::eStorageImage: write_storage_image(set, manager, binding); break;
				case vk::DescriptorType::eUniformBuffer: write_uniform_buffer(set, manager, binding); break;
				case vk::DescriptorType::eStorageBuffer: write_storage_buffer(set, manager, binding); break;
				case vk::DescriptorType::eUniformTexelBuffer: write_uniform_texel_buffer(set, manager, binding); break;
				case vk::DescriptorType::eStorageTexelBuffer: write_storage_texel_buffer(set, manager, binding); break;
				default: break;
			}
		}

		API->current_command_buffer()->bindDescriptorSets(bind_point, m_layout->layout(), 0, set, {});
		return *this;
	}

	void VulkanPipeline::bind()
	{
		API->m_state_manager->bind(this);
	}

	uint64_t VulkanGraphicsPipeline::KeyHasher::operator()(const Key& key) const
	{
		return memory_hash(&key, sizeof(key));
	}

	vk::Pipeline VulkanGraphicsPipeline::find_or_create_pipeline(VulkanStateManager* manager)
	{
		auto rt = API->m_state_manager->render_target();

		Key key;
		key.pass               = manager->render_target()->m_render_pass;
		key.primitive_topology = manager->primitive_topology();
		key.polygon_mode       = manager->polygon_mode();
		key.cull_mode          = manager->cull_mode();
		key.front_face         = manager->front_face();

		auto& pipeline = m_pipelines[key];

		if (pipeline)
		{
			return pipeline;
		}

		static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));
		static vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

		m_input_assembly.setTopology(key.primitive_topology);
		m_rasterizer.setPolygonMode(key.polygon_mode);
		m_rasterizer.setCullMode(key.cull_mode);
		m_rasterizer.setFrontFace(key.front_face);

		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);
		vk::PipelineMultisampleStateCreateInfo multisampling;


		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, &m_vertex_input, &m_input_assembly, nullptr, &viewport_state,
		                                             &m_rasterizer, &multisampling, &m_depth_stencil, &m_color_blending,
		                                             &dynamic_state, layout()->layout(), rt->m_render_pass->render_pass(), 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const RHIGraphicsPipelineInitializer* pipeline)
	    : VulkanPipeline(pipeline->parameters, pipeline->parameters_count, parse_stages_flags(pipeline))
	{
		static auto get_stencil_op_state = [](const RHIStencilTest& pipeline) {
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

		m_input_assembly.primitiveRestartEnable = vk::False;
		m_input_assembly.topology               = vk::PrimitiveTopology::eTriangleList;
		m_rasterizer.polygonMode                = vk::PolygonMode::eFill;
		m_rasterizer.cullMode                   = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace                  = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth                  = 1.f;

		auto stencil_state = get_stencil_op_state(pipeline->stencil);
		m_depth_stencil.setDepthTestEnable(pipeline->depth.enable)
		        .setDepthWriteEnable(pipeline->depth.write_enable)
		        .setDepthBoundsTestEnable(vk::False)
		        .setDepthCompareOp(VulkanEnums::compare_of(pipeline->depth.func))
		        .setMinDepthBounds(0.f)
		        .setMaxDepthBounds(0.f)
		        .setStencilTestEnable(pipeline->stencil.enable)
		        .setFront(stencil_state)
		        .setBack(stencil_state);

		for (auto& attachment : m_color_blend_attachment)
		{
			attachment.setBlendEnable(pipeline->blending.enable)
			        .setSrcColorBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.src_color_func, false))
			        .setDstColorBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.dst_color_func, false))
			        .setColorBlendOp(VulkanEnums::blend_of(pipeline->blending.color_op))
			        .setSrcAlphaBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.src_alpha_func, true))
			        .setDstAlphaBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.dst_alpha_func, true))
			        .setAlphaBlendOp(VulkanEnums::blend_of(pipeline->blending.alpha_op));

			vk::ColorComponentFlags color_mask;

			{
				EnumerateType R = enum_value_of(RHIColorComponent::R);
				EnumerateType G = enum_value_of(RHIColorComponent::G);
				EnumerateType B = enum_value_of(RHIColorComponent::B);
				EnumerateType A = enum_value_of(RHIColorComponent::A);

				auto mask = enum_value_of(pipeline->blending.write_mask);

				if ((mask & R) == R)
				{
					color_mask |= vk::ColorComponentFlagBits::eR;
				}
				if ((mask & G) == G)
				{
					color_mask |= vk::ColorComponentFlagBits::eG;
				}
				if ((mask & B) == B)
				{
					color_mask |= vk::ColorComponentFlagBits::eB;
				}
				if ((mask & A) == A)
				{
					color_mask |= vk::ColorComponentFlagBits::eA;
				}

				attachment.setColorWriteMask(color_mask);
			}
		}

		m_color_blending.setAttachments(m_color_blend_attachment).setLogicOpEnable(false);

		m_binding_description.reserve(pipeline->vertex_attributes_count);
		m_attribute_description.reserve(pipeline->vertex_attributes_count);

		for (size_t index = 0; index < pipeline->vertex_attributes_count; ++index)
		{
			auto& attribute   = pipeline->vertex_attributes[index];
			uint32_t stream   = static_cast<uint32_t>(attribute.stream_index);
			vk::Format format = VulkanEnums::vertex_format_of(attribute.type);

			{
				// Find and setup binding description
				bool found = false;
				for (auto& desc : m_binding_description)
				{
					if (desc.binding == stream)
					{
						vk::VertexInputRate rate = VulkanEnums::input_rate_of(attribute.rate);

						if (desc.inputRate != rate)
						{
							const char* name = rate == vk::VertexInputRate::eVertex ? "Vertex" : "Instance";
							error_log("Vulkan", "Stream '%d' already used for '%s' rate, but attribute '%zu' has rate '%s'", name,
							          index, name);
						}

						found = true;
						break;
					}
				}

				if (!found)
				{
					m_binding_description.emplace_back();
					auto& desc     = m_binding_description.back();
					desc.binding   = attribute.stream_index;
					desc.stride    = 0;
					desc.inputRate = VulkanEnums::input_rate_of(attribute.rate);
				}
			}

			{
				m_attribute_description.emplace_back();
				vk::VertexInputAttributeDescription& description = m_attribute_description.back();
				description.binding  = static_cast<decltype(description.binding)>(attribute.stream_index);
				description.location = static_cast<decltype(description.location)>(attribute.location);
				description.offset   = attribute.offset;
				description.format   = format;
			}
		}

		m_vertex_input.setVertexBindingDescriptions(m_binding_description);
		m_vertex_input.setVertexAttributeDescriptions(m_attribute_description);

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

	bool VulkanGraphicsPipeline::is_dirty_vertex_strides(VulkanStateManager* manager)
	{
		vk::VertexInputBindingDescription* description =
		        const_cast<vk::VertexInputBindingDescription*>(m_vertex_input.pVertexBindingDescriptions);
		size_t count = m_vertex_input.vertexBindingDescriptionCount;

		bool dirty = false;

		while (count > 0)
		{
			uint16_t stride = manager->vertex_buffers_stride.resource(description->binding);
			if (stride != description->stride)
			{
				description->stride = stride;
				dirty               = true;
			}

			--count;
			++description;
		}

		return dirty;
	}

	VulkanPipeline& VulkanGraphicsPipeline::flush(VulkanStateManager* manager)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");
		auto dirty_flags = VulkanStateManager::RenderTarget | VulkanStateManager::Pipeline |
		                   VulkanStateManager::PrimitiveTopology | VulkanStateManager::PolygonMode |
		                   VulkanStateManager::CullMode | VulkanStateManager::FrontFace;

		bool is_dirty = is_dirty_vertex_strides(manager);

		if (manager->is_dirty(dirty_flags) || is_dirty)
		{
			auto cmd              = API->current_command_buffer();
			auto current_pipeline = find_or_create_pipeline(manager);
			cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
		}

		VulkanPipeline::flush_descriptors(manager, vk::PipelineBindPoint::eGraphics);
		return *this;
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		for (auto& pipeline : m_pipelines)
		{
			DESTROY_CALL(destroyPipeline, pipeline.second);
		}
	}

	uint64_t VulkanMeshPipeline::KeyHasher::operator()(const Key& key) const
	{
		return memory_hash(&key, sizeof(key));
	}

	VulkanMeshPipeline::VulkanMeshPipeline(const RHIMeshPipelineInitializer* pipeline)
	    : VulkanPipeline(pipeline->parameters, pipeline->parameters_count, parse_stages_flags(pipeline))
	{
		static auto get_stencil_op_state = [](const RHIStencilTest& pipeline) {
			vk::StencilOpState out_state;
			out_state.setReference(0)
			        .setWriteMask(pipeline.write_mask)
			        .setCompareMask(pipeline.compare_mask)
			        .setCompareOp(VulkanEnums::compare_of(pipeline.compare))
			        .setFailOp(VulkanEnums::stencil_of(pipeline.fail))
			        .setPassOp(VulkanEnums::stencil_of(pipeline.depth_pass))
			        .setDepthFailOp(VulkanEnums::stencil_of(pipeline.depth_fail));
			return out_state;
		};

		m_rasterizer.polygonMode = vk::PolygonMode::eFill;
		m_rasterizer.cullMode    = vk::CullModeFlagBits::eNone;
		m_rasterizer.frontFace   = vk::FrontFace::eClockwise;
		m_rasterizer.lineWidth   = 1.f;

		auto stencil_state = get_stencil_op_state(pipeline->stencil);
		m_depth_stencil.setDepthTestEnable(pipeline->depth.enable)
		        .setDepthWriteEnable(pipeline->depth.write_enable)
		        .setDepthBoundsTestEnable(vk::False)
		        .setDepthCompareOp(VulkanEnums::compare_of(pipeline->depth.func))
		        .setMinDepthBounds(0.f)
		        .setMaxDepthBounds(0.f)
		        .setStencilTestEnable(pipeline->stencil.enable)
		        .setFront(stencil_state)
		        .setBack(stencil_state);

		for (auto& attachment : m_color_blend_attachment)
		{
			attachment.setBlendEnable(pipeline->blending.enable)
			        .setSrcColorBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.src_color_func, false))
			        .setDstColorBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.dst_color_func, false))
			        .setColorBlendOp(VulkanEnums::blend_of(pipeline->blending.color_op))
			        .setSrcAlphaBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.src_alpha_func, true))
			        .setDstAlphaBlendFactor(VulkanEnums::blend_func_of(pipeline->blending.dst_alpha_func, true))
			        .setAlphaBlendOp(VulkanEnums::blend_of(pipeline->blending.alpha_op));

			vk::ColorComponentFlags color_mask;

			{
				EnumerateType R = enum_value_of(RHIColorComponent::R);
				EnumerateType G = enum_value_of(RHIColorComponent::G);
				EnumerateType B = enum_value_of(RHIColorComponent::B);
				EnumerateType A = enum_value_of(RHIColorComponent::A);

				auto mask = enum_value_of(pipeline->blending.write_mask);

				if ((mask & R) == R)
				{
					color_mask |= vk::ColorComponentFlagBits::eR;
				}
				if ((mask & G) == G)
				{
					color_mask |= vk::ColorComponentFlagBits::eG;
				}
				if ((mask & B) == B)
				{
					color_mask |= vk::ColorComponentFlagBits::eB;
				}
				if ((mask & A) == A)
				{
					color_mask |= vk::ColorComponentFlagBits::eA;
				}

				attachment.setColorWriteMask(color_mask);
			}
		}

		m_color_blending.setAttachments(m_color_blend_attachment).setLogicOpEnable(false);

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
		auto rt = API->m_state_manager->render_target();

		Key key;
		key.pass         = manager->render_target()->m_render_pass;
		key.polygon_mode = manager->polygon_mode();
		key.cull_mode    = manager->cull_mode();
		key.front_face   = manager->front_face();

		auto& pipeline = m_pipelines[key];

		if (pipeline)
		{
			return pipeline;
		}

		static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));
		static vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

		m_rasterizer.setPolygonMode(key.polygon_mode);
		m_rasterizer.setCullMode(key.cull_mode);
		m_rasterizer.setFrontFace(key.front_face);

		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);
		vk::PipelineMultisampleStateCreateInfo multisampling;

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_stages, nullptr, nullptr, nullptr, &viewport_state, &m_rasterizer,
		                                             &multisampling, &m_depth_stencil, &m_color_blending, &dynamic_state,
		                                             layout()->layout(), rt->m_render_pass->render_pass(), 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanMeshPipeline& VulkanMeshPipeline::flush(VulkanStateManager* manager)
	{
		trinex_profile_cpu_n("VulkanGraphicsPipeline::flush");
		auto dirty_flags = VulkanStateManager::RenderTarget | VulkanStateManager::Pipeline | VulkanStateManager::PolygonMode |
		                   VulkanStateManager::CullMode | VulkanStateManager::FrontFace;

		if (manager->is_dirty(dirty_flags))
		{
			auto cmd              = API->current_command_buffer();
			auto current_pipeline = find_or_create_pipeline(manager);
			cmd->bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
		}

		VulkanPipeline::flush_descriptors(manager, vk::PipelineBindPoint::eGraphics);
		return *this;
	}

	VulkanMeshPipeline::~VulkanMeshPipeline()
	{
		for (auto& pipeline : m_pipelines)
		{
			DESTROY_CALL(destroyPipeline, pipeline.second);
		}
	}

	VulkanComputePipeline::VulkanComputePipeline(const RHIComputePipelineInitializer* pipeline)
	    : VulkanPipeline(pipeline->parameters, pipeline->parameters_count, vk::ShaderStageFlagBits::eCompute)
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

		vk::ComputePipelineCreateInfo info({}, stage, layout()->layout());

		auto result = API->m_device.createComputePipeline({}, info);
		if (result.result != vk::Result::eSuccess)
		{
			error_log("VulkanComputePipeline", "Failed to create pipeline");
		}

		m_pipeline = result.value;
	}

	VulkanPipeline& VulkanComputePipeline::flush(VulkanStateManager* manager)
	{
		if (manager->is_dirty(VulkanStateManager::Pipeline))
		{
			auto cmd = API->current_command_buffer();
			cmd->bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
		}

		VulkanPipeline::flush_descriptors(manager, vk::PipelineBindPoint::eCompute);
		return *this;
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		DESTROY_CALL(destroyPipeline, m_pipeline);
	}

	RHIPipeline* VulkanAPI::create_graphics_pipeline(const RHIGraphicsPipelineInitializer* pipeline)
	{
		return new VulkanGraphicsPipeline(pipeline);
	}

	RHIPipeline* VulkanAPI::create_mesh_pipeline(const RHIMeshPipelineInitializer* pipeline)
	{
		return new VulkanMeshPipeline(pipeline);
	}

	RHIPipeline* VulkanAPI::create_compute_pipeline(const RHIComputePipelineInitializer* pipeline)
	{
		return new VulkanComputePipeline(pipeline);
	}
}// namespace Engine
