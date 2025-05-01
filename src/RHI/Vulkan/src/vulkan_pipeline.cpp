#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_descript_set_layout.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
	static FORCE_INLINE void create_descriptor_layout_internal(const Pipeline* pipeline,
	                                                           Vector<vk::DescriptorSetLayoutBinding>& out,
	                                                           VulkanDescriptorSetLayout& descriptor_set_layout,
	                                                           vk::ShaderStageFlags stages)
	{
		auto push_layout_binding = [&out, &descriptor_set_layout, stages](BindLocation location, vk::DescriptorType type,
		                                                                  byte VulkanDescriptorSetLayout::* counter) {
			for (auto& entry : out)
			{
				if (entry.binding == location.binding && entry.descriptorType == type)
				{
					entry.stageFlags |= stages;
					return;
				}
			}

			out.push_back(vk::DescriptorSetLayoutBinding(location.binding, type, 1, stages, nullptr));
			++(descriptor_set_layout.*counter);
		};

		for (auto& [name, param] : pipeline->parameters)
		{
			namespace MP = MaterialParameters;

			constexpr ShaderParameterType combined_image_sampler(ShaderParameterType::META_Texture |
			                                                     ShaderParameterType::META_Sampler);

			if ((param.type & combined_image_sampler) == combined_image_sampler)
			{
				push_layout_binding(param.location, vk::DescriptorType::eCombinedImageSampler,
				                    &VulkanDescriptorSetLayout::combined_image_sampler);
			}
			else if ((param.type & ShaderParameterType::META_Texture) == ShaderParameterType::META_Texture)
			{
				push_layout_binding(param.location, vk::DescriptorType::eSampledImage, &VulkanDescriptorSetLayout::textures);
			}
			else if ((param.type & ShaderParameterType::META_Sampler) == ShaderParameterType::META_Sampler)
			{
				push_layout_binding(param.location, vk::DescriptorType::eSampler, &VulkanDescriptorSetLayout::samplers);
			}
			else if ((param.type & ShaderParameterType::META_UniformBuffer) == ShaderParameterType::META_UniformBuffer)
			{
				push_layout_binding(param.location, vk::DescriptorType::eUniformBuffer,
				                    &VulkanDescriptorSetLayout::uniform_buffers);
			}
			else if ((param.type & ShaderParameterType::META_RWTexture) == ShaderParameterType::META_RWTexture)
			{
				push_layout_binding(param.location, vk::DescriptorType::eStorageImage,
				                    &VulkanDescriptorSetLayout::uniform_buffers);
			}
		}
	}

	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const Pipeline* pipeline)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);
		ShaderType types            = pipeline->shader_types();

		if ((types & ShaderType::Vertex) == ShaderType::Vertex)
		{
			stages |= vk::ShaderStageFlagBits::eVertex;
		}

		if ((types & ShaderType::TessellationControl) == ShaderType::TessellationControl)
		{
			stages |= vk::ShaderStageFlagBits::eTessellationControl;
		}

		if ((types & ShaderType::Tessellation) == ShaderType::Tessellation)
		{
			stages |= vk::ShaderStageFlagBits::eTessellationEvaluation;
		}

		if ((types & ShaderType::Geometry) == ShaderType::Geometry)
		{
			stages |= vk::ShaderStageFlagBits::eGeometry;
		}

		if ((types & ShaderType::Fragment) == ShaderType::Fragment)
		{
			stages |= vk::ShaderStageFlagBits::eFragment;
		}

		if ((types & ShaderType::Compute) == ShaderType::Compute)
		{
			stages |= vk::ShaderStageFlagBits::eCompute;
		}

		return stages;
	}

	VulkanPipeline& VulkanPipeline::create_descriptor_set_layout(const Pipeline* pipeline)
	{
		Vector<vk::DescriptorSetLayoutBinding> layout_bindings;
		m_descriptor_set_layout = new VulkanDescriptorSetLayout();

		auto stages = parse_stages_flags(pipeline);
		create_descriptor_layout_internal(pipeline, layout_bindings, *m_descriptor_set_layout, stages);

		if (!layout_bindings.empty())
		{
			vk::DescriptorSetLayoutCreateInfo layout_info({}, layout_bindings);
			m_descriptor_set_layout->layout = API->m_device.createDescriptorSetLayout(layout_info);
		}

		return *this;
	}

	static inline vk::ShaderModule& vulkan_shader_of(Shader* shader)
	{
		return static_cast<VulkanShaderBase*>(shader->rhi_shader())->m_shader;
	}

	bool VulkanPipeline::create_pipeline_layout()
	{
		vk::ArrayProxyNoTemporaries<vk::DescriptorSetLayout> layouts =
		        m_descriptor_set_layout->has_layouts() ? m_descriptor_set_layout->layout
		                                               : vk::ArrayProxyNoTemporaries<vk::DescriptorSetLayout>{};
		vk::PipelineLayoutCreateInfo pipeline_layout_info({}, layouts);
		m_pipeline_layout = API->m_device.createPipelineLayout(pipeline_layout_info);
		return true;
	}

	VulkanDescriptorSet* VulkanPipeline::current_descriptor_set()
	{
		return m_descriptor_set_layout->has_layouts() ? m_descriptor_set : nullptr;
	}

	VulkanPipeline::~VulkanPipeline()
	{
		m_descriptor_set_layout->release();
		DESTROY_CALL(destroyPipelineLayout, m_pipeline_layout);
	}

	VulkanPipeline& VulkanPipeline::bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location,
	                                                    vk::DescriptorType type)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_uniform_buffer(info, location, type);
		}
		return *this;
	}

	VulkanPipeline& VulkanPipeline::bind_sampler(VulkanSampler* sampler, BindLocation location)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_sampler(sampler, location);
		}
		return *this;
	}

	VulkanPipeline& VulkanPipeline::bind_srv(VulkanSRV* srv, byte location, VulkanSampler* sampler)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_srv(srv, location, sampler);
		}
		return *this;
	}

	VulkanPipeline& VulkanPipeline::VulkanPipeline::bind_uav(VulkanUAV* uav, byte location)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_uav(uav, location);
		}
		return *this;
	}

	VulkanPipeline& VulkanPipeline::bind_descriptor_set(vk::PipelineBindPoint point)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind(m_pipeline_layout, point);
		}
		return *this;
	}

	// GRAPHICS PIPELINE
	static FORCE_INLINE vk::FrontFace invert_face(vk::FrontFace face)
	{
		if (face == vk::FrontFace::eClockwise)
		{
			return vk::FrontFace::eCounterClockwise;
		}
		return vk::FrontFace::eClockwise;
	}

	VulkanGraphicsPipeline::State& VulkanGraphicsPipeline::State::init(const GraphicsPipeline* in_state)
	{
		static auto get_stencil_op_state = [](const GraphicsPipelineDescription::StencilTestInfo& in_state) {
			vk::StencilOpState out_state;
			out_state.setReference(0)
			        .setWriteMask(in_state.write_mask)
			        .setCompareMask(in_state.compare_mask)
			        .setCompareOp(m_compare_funcs[static_cast<EnumerateType>(in_state.compare)])
			        .setFailOp(m_stencil_ops[static_cast<EnumerateType>(in_state.fail)])
			        .setPassOp(m_stencil_ops[static_cast<EnumerateType>(in_state.depth_pass)])
			        .setDepthFailOp(m_stencil_ops[static_cast<EnumerateType>(in_state.depth_fail)]);
			return out_state;
		};

		input_assembly.primitiveRestartEnable = vk::False;
		input_assembly.topology = m_primitive_topologies[static_cast<EnumerateType>(in_state->input_assembly.primitive_topology)];


		if (API->m_features.fillModeNonSolid)
		{
			rasterizer.setPolygonMode(m_poligon_modes[static_cast<EnumerateType>(in_state->rasterizer.polygon_mode)]);
		}
		else
		{
			if (in_state->rasterizer.polygon_mode != PolygonMode::Fill)
			{
				Name name = Refl::Enum::static_find("Engine::PolygoneMode", Refl::FindFlags::IsRequired)
				                    ->entry(static_cast<EnumerateType>(in_state->rasterizer.polygon_mode))
				                    ->name;
				error_log("Vulkan", "Polygon mode '%s' is not supported on this device. Force set it to PoligoneMode::Fill",
				          name.c_str());
			}

			rasterizer.setPolygonMode(vk::PolygonMode::eFill);
		}

		rasterizer.setCullMode(m_cull_modes[static_cast<EnumerateType>(in_state->rasterizer.cull_mode)])
		        .setLineWidth(API->m_features.wideLines ? in_state->rasterizer.line_width : 1.f)
		        .setFrontFace(m_front_faces[static_cast<EnumerateType>(in_state->rasterizer.front_face)]);

		multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		        .setSampleShadingEnable(VK_FALSE)
		        .setMinSampleShading(0.0f)
		        .setAlphaToCoverageEnable(VK_FALSE)
		        .setAlphaToOneEnable(VK_FALSE);


		auto stencil_state = get_stencil_op_state(in_state->stencil_test);
		depth_stencil.setDepthTestEnable(in_state->depth_test.enable)
		        .setDepthWriteEnable(in_state->depth_test.write_enable)
		        .setDepthBoundsTestEnable(vk::False)
		        .setDepthCompareOp(m_compare_funcs[static_cast<EnumerateType>(in_state->depth_test.func)])
		        .setMinDepthBounds(0.f)
		        .setMaxDepthBounds(0.f)
		        .setStencilTestEnable(in_state->stencil_test.enable)
		        .setFront(stencil_state)
		        .setBack(stencil_state);

		for (auto& attachment : color_blend_attachment)
		{
			attachment.setBlendEnable(in_state->color_blending.enable)
			        .setSrcColorBlendFactor(VulkanEnums::blend_func_of(in_state->color_blending.src_color_func, false))
			        .setDstColorBlendFactor(VulkanEnums::blend_func_of(in_state->color_blending.dst_color_func, false))
			        .setColorBlendOp(m_blend_ops[static_cast<EnumerateType>(in_state->color_blending.color_op)])
			        .setSrcAlphaBlendFactor(VulkanEnums::blend_func_of(in_state->color_blending.src_alpha_func, true))
			        .setDstAlphaBlendFactor(VulkanEnums::blend_func_of(in_state->color_blending.dst_alpha_func, true))
			        .setAlphaBlendOp(m_blend_ops[static_cast<EnumerateType>(in_state->color_blending.alpha_op)]);

			vk::ColorComponentFlags color_mask;

			{
				EnumerateType R = enum_value_of(ColorComponent::R);
				EnumerateType G = enum_value_of(ColorComponent::G);
				EnumerateType B = enum_value_of(ColorComponent::B);
				EnumerateType A = enum_value_of(ColorComponent::A);

				auto mask = enum_value_of(in_state->color_blending.color_mask);

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

		color_blending.setAttachments(color_blend_attachment).setLogicOpEnable(false);

		if (VertexShader* vertex_shader = in_state->vertex_shader())
		{
			vertex_input.setVertexBindingDescriptions(
			        vertex_shader->rhi_shader()->as<VulkanVertexShader>()->m_binding_description);
			vertex_input.setVertexAttributeDescriptions(
			        vertex_shader->rhi_shader()->as<VulkanVertexShader>()->m_attribute_description);
		}

		struct Stage {
			ShaderType::Enum type;
			vk::ShaderStageFlagBits vk_type;
			const char* name;
		};

		static Stage graphics_stages[] = {
		        {ShaderType::Vertex, vk::ShaderStageFlagBits::eVertex, "Vertex"},
		        {ShaderType::TessellationControl, vk::ShaderStageFlagBits::eTessellationControl, "Tesselation Control"},
		        {ShaderType::Tessellation, vk::ShaderStageFlagBits::eTessellationEvaluation, "Tesselation"},
		        {ShaderType::Geometry, vk::ShaderStageFlagBits::eGeometry, "Geometry"},
		        {ShaderType::Fragment, vk::ShaderStageFlagBits::eFragment, "Fragment"},
		};

		for (Stage& stage : graphics_stages)
		{
			if (Shader* shader = in_state->shader(stage.type))
			{
				if (!shader->rhi_shader())
				{
					String msg = Strings::format("Cannot init pipeline, because '{}' shader is not valid", stage.name);
					throw EngineException(msg);
				}

				stages.emplace_back(vk::PipelineShaderStageCreateFlags(), stage.vk_type, vulkan_shader_of(shader), "main");
			}
		}

		return *this;
	}

	VulkanGraphicsPipeline::State& VulkanGraphicsPipeline::State::flip_viewport()
	{
		rasterizer.frontFace = invert_face(rasterizer.frontFace);
		return *this;
	}

	vk::Pipeline VulkanGraphicsPipeline::find_or_create_pipeline()
	{
		auto rt = API->m_state.render_target();

		Identifier identifier            = reinterpret_cast<Identifier>(rt->m_render_pass);
		VulkanViewportMode viewport_mode = API->m_state.m_viewport_mode;

		if (viewport_mode == VulkanViewportMode::Flipped)
		{
			++identifier;
		}

		auto& pipeline = m_pipelines[identifier];

		if (pipeline)
		{
			return pipeline;
		}

		static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
		static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));
		static vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

		if (viewport_mode == VulkanViewportMode::Flipped)
			m_state.flip_viewport();

		vk::DynamicState dynamic_state_params[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
		vk::PipelineDynamicStateCreateInfo dynamic_state({}, 2, &dynamic_state_params[0]);

		vk::GraphicsPipelineCreateInfo pipeline_info({}, m_state.stages, &m_state.vertex_input, &m_state.input_assembly, nullptr,
		                                             &viewport_state, &m_state.rasterizer, &m_state.multisampling,
		                                             &m_state.depth_stencil, &m_state.color_blending, &dynamic_state,
		                                             m_pipeline_layout, rt->m_render_pass->m_render_pass, 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (viewport_mode == VulkanViewportMode::Flipped)
			m_state.flip_viewport();

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	bool VulkanGraphicsPipeline::create(const GraphicsPipeline* pipeline)
	{
		m_state.init(pipeline);
		create_descriptor_set_layout(pipeline);
		create_pipeline_layout();
		return true;
	}

	void VulkanGraphicsPipeline::bind()
	{
		auto cmd = API->current_command_buffer();
		{
			auto current_pipeline = find_or_create_pipeline();
			if (API->m_state.m_vk_pipeline != current_pipeline)
			{
				cmd->m_cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
				cmd->add_object(this);
				API->m_state.m_pipeline    = this;
				API->m_state.m_vk_pipeline = current_pipeline;
			}
		}

		if (m_descriptor_set_layout->has_layouts())
		{
			m_descriptor_set = cmd->descriptor_set_manager()->allocate_descriptor_set(m_descriptor_set_layout);
		}
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		for (auto& pipeline : m_pipelines)
		{
			DESTROY_CALL(destroyPipeline, pipeline.second);
		}
	}

	bool VulkanComputePipeline::create(const ComputePipeline* pipeline)
	{
		create_descriptor_set_layout(pipeline);
		create_pipeline_layout();

		vk::PipelineShaderStageCreateInfo stage;
		if (Shader* shader = pipeline->compute_shader())
		{
			if (!shader->rhi_shader())
			{
				error_log("VulkanPipeline", "Cannot init pipeline, because 'Compute' shader is not valid");
			}

			stage.setStage(vk::ShaderStageFlagBits::eCompute).setModule(vulkan_shader_of(shader)).setPName("main");
		}
		else
		{
			error_log("VulkanPipeline", "Cannot init pipeline, because 'Compute' shader is not valid");
			return false;
		}

		vk::ComputePipelineCreateInfo info({}, stage, m_pipeline_layout);

		auto result = API->m_device.createComputePipeline({}, info);
		if (result.result != vk::Result::eSuccess)
		{
			error_log("VulkanComputePipeline", "Failed to create pipeline");
			return false;
		}

		m_pipeline = result.value;
		return true;
	}

	void VulkanComputePipeline::bind()
	{
		auto cmd = API->current_command_buffer();
		{
			if (API->m_state.m_vk_pipeline != m_pipeline)
			{
				cmd->m_cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_pipeline);
				cmd->add_object(this);
				API->m_state.m_pipeline    = this;
				API->m_state.m_vk_pipeline = m_pipeline;
			}
		}

		if (m_descriptor_set_layout->has_layouts())
		{
			m_descriptor_set = cmd->descriptor_set_manager()->allocate_descriptor_set(m_descriptor_set_layout);
		}
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
		DESTROY_CALL(destroyPipeline, m_pipeline);
	}

	RHI_Pipeline* VulkanAPI::create_graphics_pipeline(const GraphicsPipeline* pipeline)
	{
		auto vulkan_pipeline = new VulkanGraphicsPipeline();
		if (vulkan_pipeline->create(pipeline))
		{
			return vulkan_pipeline;
		}
		delete vulkan_pipeline;
		return nullptr;
	}

	RHI_Pipeline* VulkanAPI::create_compute_pipeline(const ComputePipeline* pipeline)
	{
		auto vulkan_pipeline = new VulkanComputePipeline();
		if (vulkan_pipeline->create(pipeline))
		{
			return vulkan_pipeline;
		}
		delete vulkan_pipeline;
		return nullptr;
	}
}// namespace Engine
