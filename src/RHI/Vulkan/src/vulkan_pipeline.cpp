#include <Core/enum.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_descript_set_layout.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
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
	/////////////////////// PIPELINE STATE PARSING ///////////////////////

	static FORCE_INLINE vk::FrontFace invert_face(vk::FrontFace face)
	{
		if (face == vk::FrontFace::eClockwise)
		{
			return vk::FrontFace::eCounterClockwise;
		}
		return vk::FrontFace::eClockwise;
	}

	VulkanPipeline::State& VulkanPipeline::State::init(const Pipeline* in_state, bool with_flipped_viewport)
	{
		static auto get_stencil_op_state = [](const Pipeline::StencilTestInfo& in_state) {
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
		input_assembly.topology =
				m_primitive_topologies[static_cast<EnumerateType>(in_state->input_assembly.primitive_topology)];


		if (API->m_features.fillModeNonSolid)
		{
			rasterizer.setPolygonMode(m_poligon_modes[static_cast<EnumerateType>(in_state->rasterizer.polygon_mode)]);
		}
		else
		{
			if (in_state->rasterizer.polygon_mode != PolygonMode::Fill)
			{
				Name name = Enum::static_find("Engine::PolygoneMode", true)
									->entry(static_cast<EnumerateType>(in_state->rasterizer.polygon_mode))
									->name;
				error_log("Vulkan",
						  "Polygon mode '%s' is not supported on this device. Force set it to PoligoneMode::Fill",
						  name.c_str());
			}

			rasterizer.setPolygonMode(vk::PolygonMode::eFill);
		}

		rasterizer.setCullMode(m_cull_modes[static_cast<EnumerateType>(in_state->rasterizer.cull_mode)])
				.setLineWidth(API->m_features.wideLines ? in_state->rasterizer.line_width : 1.f);

		if (with_flipped_viewport)
		{
			rasterizer.setFrontFace(
					invert_face(m_front_faces[static_cast<EnumerateType>(in_state->rasterizer.front_face)]));
		}
		else
		{
			rasterizer.setFrontFace(m_front_faces[static_cast<EnumerateType>(in_state->rasterizer.front_face)]);
		}

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


		auto color_attachments_count =
				API->m_state.m_render_target->state()->m_render_pass->m_color_attachment_references.size();
		color_blend_attachment.resize(color_attachments_count);


		for (auto& attachment : color_blend_attachment)
		{
			attachment.setBlendEnable(in_state->color_blending.enable)
					.setSrcColorBlendFactor(get_type(in_state->color_blending.src_color_func, false))
					.setDstColorBlendFactor(get_type(in_state->color_blending.dst_color_func, false))
					.setColorBlendOp(m_blend_ops[static_cast<EnumerateType>(in_state->color_blending.color_op)])
					.setSrcAlphaBlendFactor(get_type(in_state->color_blending.src_alpha_func, true))
					.setDstAlphaBlendFactor(get_type(in_state->color_blending.dst_alpha_func, true))
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
		dynamic_state_info.setDynamicStates(API->m_dynamic_states);
		return *this;
	}

	/////////////////////// PIPELINE STATE PARSING END ///////////////////////

	static FORCE_INLINE void create_descriptor_layout_internal(const Pipeline* pipeline,
															   Vector<vk::DescriptorSetLayoutBinding>& out,
															   VulkanDescriptorSetLayout& descriptor_set_layout,
															   vk::ShaderStageFlags stages)
	{
		auto push_layout_binding = [&out, &descriptor_set_layout, stages](BindLocation location,
																		  vk::DescriptorType type,
																		  byte VulkanDescriptorSetLayout::*counter) {
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


		if (pipeline->global_parameters.has_parameters())
		{
			push_layout_binding(pipeline->global_parameters.bind_index(), vk::DescriptorType::eUniformBuffer,
								&VulkanDescriptorSetLayout::uniform_buffers);
		}

		if (pipeline->local_parameters.has_parameters())
		{
			push_layout_binding(pipeline->local_parameters.bind_index(), vk::DescriptorType::eUniformBuffer,
								&VulkanDescriptorSetLayout::uniform_buffers);
		}

		for (auto& [name, param] : pipeline->parameters)
		{
			switch (param.type)
			{
				case MaterialParameterType::Texture2D:
					push_layout_binding(param.location, vk::DescriptorType::eSampledImage,
										&VulkanDescriptorSetLayout::textures);
					break;
				case MaterialParameterType::Sampler:
					push_layout_binding(param.location, vk::DescriptorType::eSampler,
										&VulkanDescriptorSetLayout::samplers);
					break;
				case MaterialParameterType::CombinedImageSampler2D:
					push_layout_binding(param.location, vk::DescriptorType::eCombinedImageSampler,
										&VulkanDescriptorSetLayout::combined_image_sampler);
					break;
				default:
					break;
			}
		}
	}

	static FORCE_INLINE vk::ShaderStageFlags parse_stages_flags(const Pipeline* pipeline)
	{
		vk::ShaderStageFlags stages = vk::ShaderStageFlags(0);

		for (auto& shader : pipeline->shader_array())
		{
			if (shader)
			{
				auto type = shader->type();

				if (type != ShaderType::Undefined)
				{
					switch (type)
					{
						case ShaderType::Vertex:
							stages |= vk::ShaderStageFlagBits::eVertex;
							break;
						case ShaderType::TessellationControl:
							stages |= vk::ShaderStageFlagBits::eTessellationControl;
							break;
						case ShaderType::Tessellation:
							stages |= vk::ShaderStageFlagBits::eTessellationEvaluation;
							break;
						case ShaderType::Geometry:
							stages |= vk::ShaderStageFlagBits::eGeometry;
							break;
						case ShaderType::Fragment:
							stages |= vk::ShaderStageFlagBits::eFragment;
							break;
						case ShaderType::Compute:
							stages |= vk::ShaderStageFlagBits::eCompute;
							break;
						default:
							break;
					}
				}
			}
		}

		return stages;
	}

	VulkanPipeline& VulkanPipeline::create_descriptor_set_layout()
	{
		Vector<vk::DescriptorSetLayoutBinding> layout_bindings;

		auto stages = parse_stages_flags(m_engine_pipeline);
		create_descriptor_layout_internal(m_engine_pipeline, layout_bindings, m_descriptor_set_layout, stages);

		if (!layout_bindings.empty())
		{
			vk::DescriptorSetLayoutCreateInfo layout_info({}, layout_bindings);
			m_descriptor_set_layout.layout = API->m_device.createDescriptorSetLayout(layout_info);
		}

		return *this;
	}

#define check_shader(var, name)                                                                                        \
	if (!var->has_object())                                                                                            \
	{                                                                                                                  \
		throw EngineException("Cannot init pipeline, because " #name " shader is not valid");                          \
	}


	Vector<vk::PipelineShaderStageCreateInfo> VulkanPipeline::create_pipeline_stage_infos()
	{
		Vector<vk::PipelineShaderStageCreateInfo> pipeline_stage_create_infos;

		if (VertexShader* vertex_shader = m_engine_pipeline->vertex_shader())
		{
			check_shader(vertex_shader, vertex);
			pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
													 vk::ShaderStageFlagBits::eVertex,
													 vertex_shader->rhi_object<VulkanVertexShader>()->m_shader, "main");
		}

		if (TessellationControlShader* tsc_shader = m_engine_pipeline->tessellation_control_shader())
		{
			check_shader(tsc_shader, tessellation control);
			pipeline_stage_create_infos.emplace_back(
					vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eTessellationControl,
					tsc_shader->rhi_object<VulkanTessellationControlShader>()->m_shader, "main");
		}

		if (TessellationShader* ts_shader = m_engine_pipeline->tessellation_shader())
		{
			check_shader(ts_shader, tessellation);
			pipeline_stage_create_infos.emplace_back(
					vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eTessellationEvaluation,
					ts_shader->rhi_object<VulkanTessellationShader>()->m_shader, "main");
		}

		if (GeometryShader* geo_shader = m_engine_pipeline->geometry_shader())
		{
			check_shader(geo_shader, geometry);
			pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
													 vk::ShaderStageFlagBits::eGeometry,
													 geo_shader->rhi_object<VulkanGeometryShader>()->m_shader, "main");
		}

		if (FragmentShader* fragment_shader = m_engine_pipeline->fragment_shader())
		{
			check_shader(fragment_shader, fragment);
			pipeline_stage_create_infos.emplace_back(
					vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,
					fragment_shader->rhi_object<VulkanFragmentShader>()->m_shader, "main");
		}

		if (pipeline_stage_create_infos.empty())
		{
			throw EngineException("Cannot to create pipeline without stages");
		}

		return pipeline_stage_create_infos;
	}

	vk::PipelineVertexInputStateCreateInfo VulkanPipeline::create_vertex_input_info()
	{
		if (VertexShader* vertex_shader = m_engine_pipeline->vertex_shader())
		{
			check_shader(vertex_shader, vertex);

			vk::PipelineVertexInputStateCreateInfo vertex_input_info;
			vertex_input_info.setVertexBindingDescriptions(
					vertex_shader->rhi_object<VulkanVertexShader>()->m_binding_description);
			vertex_input_info.setVertexAttributeDescriptions(
					vertex_shader->rhi_object<VulkanVertexShader>()->m_attribute_description);
			return vertex_input_info;
		}

		return {};
	}

	bool VulkanPipeline::create_pipeline_layout()
	{
		vk::ArrayProxyNoTemporaries<vk::DescriptorSetLayout> layouts =
				m_descriptor_set_layout.has_layouts() ? m_descriptor_set_layout.layout
													  : vk::ArrayProxyNoTemporaries<vk::DescriptorSetLayout>{};
		vk::PipelineLayoutCreateInfo pipeline_layout_info({}, layouts);
		m_pipeline_layout = API->m_device.createPipelineLayout(pipeline_layout_info);
		return true;
	}

	vk::Pipeline VulkanPipeline::find_or_create_pipeline()
	{
		Identifier identifier = reinterpret_cast<Identifier>(API->m_state.m_render_target->state()->m_render_pass);
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

		State& out_state = create_pipeline_state(viewport_mode == VulkanViewportMode::Flipped);

		auto pipeline_stage_create_infos						 = create_pipeline_stage_infos();
		vk::PipelineVertexInputStateCreateInfo vertex_input_info = create_vertex_input_info();

		vk::GraphicsPipelineCreateInfo pipeline_info(
				{}, pipeline_stage_create_infos, &vertex_input_info, &out_state.input_assembly, nullptr,
				&viewport_state, &out_state.rasterizer, &out_state.multisampling, &out_state.depth_stencil,
				&out_state.color_blending, &out_state.dynamic_state_info, m_pipeline_layout,
				API->m_state.m_render_target->state()->m_render_pass->m_render_pass, 0, {});

		auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

		if (pipeline_result.result != vk::Result::eSuccess)
		{
			throw EngineException("Failed to create pipeline");
		}

		pipeline = pipeline_result.value;
		return pipeline;
	}

	VulkanDescriptorSet* VulkanPipeline::current_descriptor_set()
	{
		return m_descriptor_set_layout.has_layouts() ? m_descriptor_sets[API->m_current_buffer][m_descriptor_set_index]
													 : nullptr;
	}

	const MaterialScalarParametersInfo& VulkanPipeline::global_parameters_info() const
	{
		return m_engine_pipeline->global_parameters;
	}

	const MaterialScalarParametersInfo& VulkanPipeline::local_parameters_info() const
	{
		return m_engine_pipeline->local_parameters;
	}


	static FORCE_INLINE void check_pipeline(const Pipeline* pipeline)
	{
		if (!pipeline)
			throw EngineException("Cannot create Vulkan Pipeline from nullptr engine pipeline");
	}

	VulkanPipeline::State& VulkanPipeline::create_pipeline_state(bool with_flipped_viewport)
	{
		static thread_local State state;
		state.init(m_engine_pipeline, with_flipped_viewport);
		return state;
	}

	bool VulkanPipeline::create(const Pipeline* pipeline)
	{
		check_pipeline(pipeline);
		m_engine_pipeline = pipeline;

		create_descriptor_set_layout();
		create_pipeline_layout();

		if (m_descriptor_set_layout.has_layouts())
		{
			m_descriptor_sets.resize(API->m_framebuffers_count);
		}
		return true;
	}


	VulkanPipeline::~VulkanPipeline()
	{
		API->wait_idle();

		for (auto& buffer : m_descriptor_sets)
		{
			for (auto& set : buffer)
			{
				VulkanDescriptorPoolManager::release_descriptor_set(set, &m_descriptor_set_layout);
			}
		}

		for (auto& pipeline : m_pipelines)
		{
			DESTROY_CALL(destroyPipeline, pipeline.second);
		}
		DESTROY_CALL(destroyPipelineLayout, m_pipeline_layout);
		m_descriptor_set_layout.destroy();
	}

	void VulkanPipeline::bind()
	{
		{
			auto current_pipeline = find_or_create_pipeline();

			if (API->m_state.m_vk_pipeline != current_pipeline)
			{
				auto cmd = API->current_command_buffer();
				cmd->m_cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, current_pipeline);
				cmd->add_object(this);
				API->m_state.m_pipeline	   = this;
				API->m_state.m_vk_pipeline = current_pipeline;
			}
		}

		if (m_last_frame != API->m_current_frame)
		{
			m_descriptor_set_index = 0;
			m_last_frame		   = API->m_current_frame;
		}
		else
		{
			++m_descriptor_set_index;
		}

		if (m_descriptor_set_layout.has_layouts())
		{
			auto& current_buffer = m_descriptor_sets[API->m_current_buffer];

			while (true)
			{
				if (current_buffer.size() <= m_descriptor_set_index)
				{
					current_buffer.push_back(
							VulkanDescriptorPoolManager::allocate_descriptor_set(&m_descriptor_set_layout));
					break;
				}

				if (current_buffer[m_descriptor_set_index]->references() <= 1)
					break;

				// Current descriptor is busy now, so, lets check next descriptor
				++m_descriptor_set_index;
			}
		}
	}

	VulkanPipeline& VulkanPipeline::bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_ssbo(ssbo, location);
		}
		return *this;
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

	VulkanPipeline& VulkanPipeline::bind_texture(VulkanTexture* texture, BindLocation location)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_texture(texture, location);
		}

		return *this;
	}

	VulkanPipeline& VulkanPipeline::bind_texture_combined(VulkanTexture* texture, VulkanSampler* sampler,
														  BindLocation location)
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind_texture_combined(texture, sampler, location);
		}
		return *this;
	}

	VulkanPipeline& VulkanPipeline::bind_descriptor_set()
	{
		if (auto current_set = current_descriptor_set())
		{
			current_set->bind(m_pipeline_layout, vk::PipelineBindPoint::eGraphics);
		}
		return *this;
	}

	RHI_Pipeline* VulkanAPI::create_pipeline(const Pipeline* pipeline)
	{
		auto vulkan_pipeline = new VulkanPipeline();
		if (vulkan_pipeline->create(pipeline))
		{
			return vulkan_pipeline;
		}
		delete vulkan_pipeline;
		return nullptr;
	}
}// namespace Engine
