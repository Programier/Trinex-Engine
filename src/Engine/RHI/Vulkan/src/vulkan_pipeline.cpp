#include <Graphics/pipeline.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    VulkanPipeline::State::State(const Pipeline& in_state)
    {
        static auto get_stencil_op_state = [](const Pipeline::StencilTestInfo::FaceInfo& in_state) {
            vk::StencilOpState out_state;
            out_state.setReference(in_state.reference)
                    .setWriteMask(in_state.write_mask)
                    .setCompareMask(in_state.compare_mask)
                    .setCompareOp(m_compare_funcs[static_cast<EnumerateType>(in_state.compare)])
                    .setFailOp(m_stencil_ops[static_cast<EnumerateType>(in_state.fail)])
                    .setPassOp(m_stencil_ops[static_cast<EnumerateType>(in_state.depth_pass)])
                    .setDepthFailOp(m_stencil_ops[static_cast<EnumerateType>(in_state.depth_fail)]);
            return out_state;
        };

        input_assembly.primitiveRestartEnable = in_state.input_assembly.primitive_restart_enable;
        input_assembly.topology = m_primitive_topologies[static_cast<EnumerateType>(in_state.input_assembly.primitive_topology)];

        rasterizer.setCullMode(m_cull_modes[static_cast<EnumerateType>(in_state.rasterizer.cull_mode)])
                .setFrontFace(m_front_faces[static_cast<EnumerateType>(in_state.rasterizer.front_face)])
                .setPolygonMode(m_poligon_modes[static_cast<EnumerateType>(in_state.rasterizer.polygon_mode)])
                .setDepthBiasSlopeFactor(in_state.rasterizer.depth_bias_slope_factor)
                .setDepthBiasClamp(in_state.rasterizer.depth_bias_clamp)
                .setDepthBiasConstantFactor(in_state.rasterizer.depth_bias_const_factor)
                .setDepthClampEnable(in_state.rasterizer.depth_clamp_enable)
                .setRasterizerDiscardEnable(in_state.rasterizer.discard_enable)
                .setDepthBiasEnable(in_state.rasterizer.depth_bias_enable)
                .setLineWidth(in_state.rasterizer.line_width);

        multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1)
                .setSampleShadingEnable(VK_FALSE)
                .setMinSampleShading(0.0f)
                .setAlphaToCoverageEnable(VK_FALSE)
                .setAlphaToOneEnable(VK_FALSE);

        depth_stencil.setDepthTestEnable(in_state.depth_test.enable)
                .setDepthWriteEnable(in_state.depth_test.write_enable)
                .setDepthBoundsTestEnable(in_state.depth_test.bound_test_enable)
                .setDepthCompareOp(m_compare_funcs[static_cast<EnumerateType>(in_state.depth_test.func)])
                .setMinDepthBounds(in_state.depth_test.min_depth_bound)
                .setMaxDepthBounds(in_state.depth_test.max_depth_bound)
                .setStencilTestEnable(in_state.stencil_test.enable)
                .setFront(get_stencil_op_state(in_state.stencil_test.front))
                .setBack(get_stencil_op_state(in_state.stencil_test.back));


        RenderPass* render_pass = in_state.render_pass();
        trinex_always_check(render_pass, "Render pass can't be nullptr!");
        color_blend_attachment.resize(render_pass->color_attachments.size());


        for (auto& attachment : color_blend_attachment)
        {
            attachment.setBlendEnable(in_state.color_blending.enable)
                    .setSrcColorBlendFactor(m_blend_factors[static_cast<EnumerateType>(in_state.color_blending.src_color_func)])
                    .setDstColorBlendFactor(m_blend_factors[static_cast<EnumerateType>(in_state.color_blending.dst_color_func)])
                    .setColorBlendOp(m_blend_ops[static_cast<EnumerateType>(in_state.color_blending.color_op)])
                    .setSrcAlphaBlendFactor(m_blend_factors[static_cast<EnumerateType>(in_state.color_blending.src_alpha_func)])
                    .setDstAlphaBlendFactor(m_blend_factors[static_cast<EnumerateType>(in_state.color_blending.dst_alpha_func)])
                    .setAlphaBlendOp(m_blend_ops[static_cast<EnumerateType>(in_state.color_blending.alpha_op)]);

            vk::ColorComponentFlags color_mask;

            {
                EnumerateType R = enum_value_of(ColorComponent::R);
                EnumerateType G = enum_value_of(ColorComponent::G);
                EnumerateType B = enum_value_of(ColorComponent::B);
                EnumerateType A = enum_value_of(ColorComponent::A);

                auto mask = enum_value_of(in_state.color_blending.color_mask);

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

        color_blending
                .setBlendConstants({in_state.color_blending.blend_constants.x, in_state.color_blending.blend_constants.y,
                                    in_state.color_blending.blend_constants.z, in_state.color_blending.blend_constants.w})
                .setAttachments(color_blend_attachment)
                .setLogicOpEnable(in_state.color_blending.logic_op_enable)
                .setLogicOp(m_logic_ops[static_cast<EnumerateType>(in_state.color_blending.logic_op)]);
    }

    static void create_descriptor_layout_internal(const Pipeline* pipeline, Vector<Vector<vk::DescriptorSetLayoutBinding>>& out,
                                                  vk::ShaderStageFlags stages)
    {
        static auto push_layout_binding = [](Vector<Vector<vk::DescriptorSetLayoutBinding>>& out, vk::ShaderStageFlags stages,
                                             BindLocation location, vk::DescriptorType type) {
            if (location.set >= out.size())
            {
                out.resize(location.set + 1);
            }

            for (auto& entry : out[location.set])
            {
                if (entry.binding == location.binding && entry.descriptorType == type)
                {
                    entry.stageFlags |= stages;
                    return;
                }
            }

            out[location.set].push_back(vk::DescriptorSetLayoutBinding(location.binding, type, 1, stages, nullptr));
        };

        if (!pipeline || stages == vk::ShaderStageFlags(0))
            return;

        if (pipeline->global_parameters.has_parameters())
        {
            push_layout_binding(out, stages, {pipeline->global_parameters.bind_index(), 0}, vk::DescriptorType::eUniformBuffer);
        }

        if (pipeline->local_parameters.has_parameters())
        {
            push_layout_binding(out, stages, {pipeline->local_parameters.bind_index(), 0}, vk::DescriptorType::eUniformBuffer);
        }


        for (auto& [name, param] : pipeline->parameters)
        {
            switch (param.type)
            {
                case MaterialParameterType::Texture2D:
                    push_layout_binding(out, stages, param.location, vk::DescriptorType::eSampledImage);
                case MaterialParameterType::Sampler:
                    push_layout_binding(out, stages, param.location, vk::DescriptorType::eSampler);
                case MaterialParameterType::CombinedImageSampler2D:
                    push_layout_binding(out, stages, param.location, vk::DescriptorType::eCombinedImageSampler);
                    break;
                default:
                    break;
            }
        }

        //        for (auto& shared_buffer : shader->ssbo)
        //        {
        //            push_layout_binding(out, stage, shared_buffer.location, vk::DescriptorType::eStorageBuffer);
        //        }
    }

    VulkanPipeline& VulkanPipeline::create_descriptor_layout(const Pipeline* pipeline)
    {
        Vector<Vector<vk::DescriptorSetLayoutBinding>> layout_bindings;
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

        create_descriptor_layout_internal(pipeline, layout_bindings, stages);


        m_descriptor_set_layout.resize(layout_bindings.size());

        Index set = 0;
        for (auto& layout : layout_bindings)
        {
            if (!layout_bindings.empty())
            {
                vk::DescriptorSetLayoutCreateInfo layout_info({}, layout);
                m_descriptor_set_layout[set] = API->m_device.createDescriptorSetLayout(layout_info);
            }
            ++set;
        }

        return *this;
    }

    VulkanDescriptorSet* VulkanPipeline::current_descriptor_set(BindingIndex set)
    {
        return m_descriptor_pool->get(set);
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

        if (pipeline->render_pass() == nullptr)
            throw EngineException("Cannot create Vulkan Pipeline without render_pass");
    }

#define check_shader(var, name)                                                                                                  \
    if (!var->has_object())                                                                                                      \
    {                                                                                                                            \
        error_log("VulkanAPI", "Cannot init pipeline, because " #name " shader is not valid");                                   \
        return false;                                                                                                            \
    }

    bool VulkanPipeline::create(const Pipeline* pipeline)
    {
        check_pipeline(pipeline);
        create_descriptor_layout(pipeline);
        m_engine_pipeline = pipeline;

        static vk::Viewport viewport(0.f, 0.f, 1280.f, 720.f, 0.0f, 1.f);
        static vk::Rect2D scissor({0, 0}, vk::Extent2D(1280, 720));

        vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
        vk::PipelineLayoutCreateInfo pipeline_layout_info({}, m_descriptor_set_layout);

        Vector<vk::PipelineShaderStageCreateInfo> pipeline_stage_create_infos;
        vk::PipelineVertexInputStateCreateInfo vertex_input_info;

        if (VertexShader* vertex_shader = pipeline->vertex_shader())
        {
            check_shader(vertex_shader, vertex);
            pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,
                                                     vertex_shader->rhi_object<VulkanVertexShader>()->m_shader, "main");
            vertex_input_info.setVertexBindingDescriptions(
                    vertex_shader->rhi_object<VulkanVertexShader>()->m_binding_description);
            vertex_input_info.setVertexAttributeDescriptions(
                    vertex_shader->rhi_object<VulkanVertexShader>()->m_attribute_description);
        }

        if (TessellationControlShader* tsc_shader = pipeline->tessellation_control_shader())
        {
            check_shader(tsc_shader, tessellation control);
            pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
                                                     vk::ShaderStageFlagBits::eTessellationControl,
                                                     tsc_shader->rhi_object<VulkanTessellationControlShader>()->m_shader, "main");
        }

        if (TessellationShader* ts_shader = pipeline->tessellation_shader())
        {
            check_shader(ts_shader, tessellation);
            pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
                                                     vk::ShaderStageFlagBits::eTessellationEvaluation,
                                                     ts_shader->rhi_object<VulkanTessellationShader>()->m_shader, "main");
        }

        if (GeometryShader* geo_shader = pipeline->geometry_shader())
        {
            check_shader(geo_shader, geometry);
            pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eGeometry,
                                                     geo_shader->rhi_object<VulkanGeometryShader>()->m_shader, "main");
        }

        if (FragmentShader* fragment_shader = pipeline->fragment_shader())
        {
            check_shader(fragment_shader, fragment);
            pipeline_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,
                                                     fragment_shader->rhi_object<VulkanFragmentShader>()->m_shader, "main");
        }

        if (pipeline_stage_create_infos.empty())
        {
            error_log("VulkanAPI", "Cannot to create pipeline without stages");
            return false;
        }


        m_pipeline_layout = API->m_device.createPipelineLayout(pipeline_layout_info);

        State out_state(*pipeline);

        vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, API->m_dynamic_states);

        RenderPass* render_pass = pipeline->render_pass();

        vk::GraphicsPipelineCreateInfo pipeline_info(
                {}, pipeline_stage_create_infos, &vertex_input_info, &out_state.input_assembly, nullptr, &viewport_state,
                &out_state.rasterizer, &out_state.multisampling, &out_state.depth_stencil, &out_state.color_blending,
                &dynamic_state_info, m_pipeline_layout, render_pass->rhi_object<VulkanRenderPass>()->m_render_pass, 0, {});

        auto pipeline_result = API->m_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
            API->m_device.destroyPipeline(m_pipeline);
            error_log("VulkanAPI", "Failed to create pipeline");
            return false;
        }

        m_pipeline = pipeline_result.value;

        if (!m_descriptor_set_layout.empty())
        {
            m_descriptor_pool = new VulkanDescriptorPool(calculate_pool_size(pipeline), this);
        }

        return true;
    }

    VulkanPipeline& VulkanPipeline::destroy()
    {
        API->wait_idle();
        DESTROY_CALL(destroyPipeline, m_pipeline);
        DESTROY_CALL(destroyPipelineLayout, m_pipeline_layout);

        for (auto& layout : m_descriptor_set_layout)
        {
            DESTROY_CALL(destroyDescriptorSetLayout, layout);
        }

        m_descriptor_set_layout.clear();

        if (m_descriptor_pool)
        {
            delete m_descriptor_pool;
            m_descriptor_pool = nullptr;
        }
        return *this;
    }

    size_t VulkanPipeline::descriptor_sets_count() const
    {
        return m_descriptor_set_layout.size();
    }

    struct PoolSizeInfo {
        uint_t samplers          = 0;
        uint_t combined_samplers = 0;
        uint_t textures          = 0;
        uint_t ubos              = 0;
        uint_t dynamic_ubos      = 0;
        uint_t ssbos             = 0;
    };

    static PoolSizeInfo calc_pool_sizes(const Pipeline* pipeline)
    {
        PoolSizeInfo out;
        uint_t stages_count = pipeline->stages_count();

        for (auto& param : pipeline->parameters)
        {
            uint_t* value = nullptr;
            switch (param.second.type)
            {
                case MaterialParameterType::Texture2D:
                    value = &(out.textures);
                case MaterialParameterType::Sampler:
                    value = &(out.samplers);
                case MaterialParameterType::CombinedImageSampler2D:
                    value = &(out.combined_samplers);
                default:
                    break;
            }

            if (value)
            {
                (*value) += stages_count;
            }
        }

        if (pipeline->global_parameters.has_parameters())
        {
            ++(out.ubos);
        }

        if (pipeline->local_parameters.has_parameters())
        {
            ++(out.ubos);
        }

        return out;
    }

    Vector<vk::DescriptorPoolSize> VulkanPipeline::calculate_pool_size(const Pipeline* pipeline)
    {
        PoolSizeInfo info = calc_pool_sizes(pipeline);
        Vector<vk::DescriptorPoolSize> pool_size;

        if (info.samplers)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eSampler, info.samplers));
        if (info.combined_samplers)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, info.combined_samplers));
        if (info.textures)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, info.textures));
        if (info.ubos)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, info.ubos));
        if (info.dynamic_ubos)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, info.dynamic_ubos));
        if (info.ssbos)
            pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, info.ssbos));

        return pool_size;
    }

    VulkanPipeline::~VulkanPipeline()
    {
        destroy();
    }

    void VulkanPipeline::bind()
    {
        if (API->m_state->m_pipeline != this)
        {
            API->current_command_buffer().bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
            API->m_state->m_pipeline = this;
        }

        if (m_descriptor_pool)
        {
            m_descriptor_pool->reset();
        }
    }

    VulkanPipeline& VulkanPipeline::submit_descriptors()
    {
        if (m_descriptor_pool)
        {
            uint_t set_index = 0;

            for (auto& set : m_descriptor_pool->get_sets_array())
            {
                set.bind(m_pipeline_layout, set_index, vk::PipelineBindPoint::eGraphics);
                ++set_index;
            }

            m_descriptor_pool->next();
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_ssbo(struct VulkanSSBO* ssbo, BindLocation location)
    {
        if (!m_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(location.set);
            current_set->bind_ssbo(ssbo, location.binding);
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_uniform_buffer(const vk::DescriptorBufferInfo& info, BindLocation location,
                                                        vk::DescriptorType type)
    {
        if (!m_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(location.set);
            current_set->bind_uniform_buffer(info, location.binding, type);
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_sampler(VulkanSampler* sampler, BindLocation location)
    {
        if (!m_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(location.set);
            current_set->bind_sampler(sampler, location.binding);
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_texture(VulkanTexture* texture, BindLocation location)
    {
        if (!m_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(location.set);
            current_set->bind_texture(texture, location.binding);
        }

        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_texture_combined(VulkanTexture* texture, VulkanSampler* sampler, BindLocation location)
    {
        if (!m_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(location.set);
            current_set->bind_texture_combined(texture, sampler, location.binding);
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
