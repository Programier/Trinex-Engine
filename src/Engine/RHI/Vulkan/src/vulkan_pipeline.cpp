#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
    static Vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    static vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, dynamic_states);


    static void push_layout_binding(Vector<Vector<vk::DescriptorSetLayoutBinding>>& out, vk::ShaderStageFlags stage,
                                    uint_t set, uint_t binding, vk::DescriptorType type)
    {
        if (set >= out.size())
        {
            out.resize(set + 1);
        }

        out[set].push_back(vk::DescriptorSetLayoutBinding(binding, type, 1, stage, nullptr));
    }

    static void create_base_descriptor_layout(ShaderBase* shader, Vector<Vector<vk::DescriptorSetLayoutBinding>>& out,
                                              vk::ShaderStageFlags stage)
    {

        for (auto& buffer : shader->uniform_buffers)
        {
            push_layout_binding(out, stage, buffer.set, buffer.binding, vk::DescriptorType::eUniformBuffer);
        }

        for (auto& texture : shader->textures)
        {
            push_layout_binding(out, stage, texture.set, texture.binding, vk::DescriptorType::eSampledImage);
        }

        for (auto& sampler : shader->samplers)
        {
            push_layout_binding(out, stage, sampler.set, sampler.binding, vk::DescriptorType::eSampler);
        }

        for (auto& combined_sampler : shader->combined_samplers)
        {
            push_layout_binding(out, stage, combined_sampler.set, combined_sampler.binding,
                                vk::DescriptorType::eCombinedImageSampler);
        }

        //        for (auto& shared_buffer : info.shared_buffers)
        //        {
        //            layout_bindings[index++] = vk::DescriptorSetLayoutBinding(
        //                    shared_buffer.binding, vk::DescriptorType::eStorageBuffer, 1,
        //                    vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, nullptr);
        //        }
    }

    static void create_vertex_descriptor_layout(const Pipeline* pipeline,
                                                Vector<Vector<vk::DescriptorSetLayoutBinding>>& out)
    {
        create_base_descriptor_layout(pipeline->vertex_shader, out, vk::ShaderStageFlagBits::eVertex);
    }

    static void create_fragment_descriptor_layout(const Pipeline* pipeline,
                                                  Vector<Vector<vk::DescriptorSetLayoutBinding>>& out)
    {
        create_base_descriptor_layout(pipeline->vertex_shader, out, vk::ShaderStageFlagBits::eFragment);
    }


    VulkanPipeline& VulkanPipeline::create_descriptor_layout(const Pipeline* pipeline)
    {
        Vector<Vector<vk::DescriptorSetLayoutBinding>> layout_bindings;

        create_vertex_descriptor_layout(pipeline, layout_bindings);
        create_fragment_descriptor_layout(pipeline, layout_bindings);


        _M_descriptor_set_layout.reserve(layout_bindings.size());


        Index set = 0;
        for (auto& layout : layout_bindings)
        {

            if (!layout_bindings.empty())
            {
                vk::DescriptorSetLayoutCreateInfo layout_info({}, layout);
                _M_descriptor_set_layout[set] = API->_M_device.createDescriptorSetLayout(layout_info);
            }
        }

        return *this;
    }


    // Use template, because PipelineState uses no-name structures
    template<typename StructType>
    static inline vk::StencilOpState get_stencil_op_state(const StructType& in_state)
    {
        vk::StencilOpState out_state;
        out_state.setReference(in_state.reference)
                .setWriteMask(in_state.write_mask)
                .setCompareMask(in_state.compare_mask)
                .setCompareOp(_M_compare_funcs[static_cast<EnumerateType>(in_state.compare)])
                .setFailOp(_M_stencil_ops[static_cast<EnumerateType>(in_state.fail)])
                .setPassOp(_M_stencil_ops[static_cast<EnumerateType>(in_state.depth_pass)])
                .setDepthFailOp(_M_stencil_ops[static_cast<EnumerateType>(in_state.depth_fail)]);
        return out_state;
    }

    static void init_pipeline_state(VulkanPipeline::State& out_state, const Pipeline& in_state)
    {
        out_state.input_assembly.primitiveRestartEnable = in_state.input_assembly.primitive_restart_enable;
        out_state.input_assembly.topology =
                _M_primitive_topologies[static_cast<EnumerateType>(in_state.input_assembly.primitive_topology)];

        out_state.rasterizer.setCullMode(_M_cull_modes[static_cast<EnumerateType>(in_state.rasterizer.cull_mode)])
                .setFrontFace(_M_front_faces[static_cast<EnumerateType>(in_state.rasterizer.cull_mode)])
                .setPolygonMode(_M_poligon_modes[static_cast<EnumerateType>(in_state.rasterizer.poligon_mode)])
                .setDepthBiasSlopeFactor(in_state.rasterizer.depth_bias_slope_factor)
                .setDepthBiasClamp(in_state.rasterizer.depth_bias_clamp)
                .setDepthBiasConstantFactor(in_state.rasterizer.depth_bias_const_factor)
                .setDepthClampEnable(in_state.rasterizer.depth_clamp_enable)
                .setRasterizerDiscardEnable(in_state.rasterizer.discard_enable)
                .setDepthBiasEnable(in_state.rasterizer.depth_bias_enable)
                .setLineWidth(in_state.rasterizer.line_width);

        out_state.multisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1)
                .setSampleShadingEnable(VK_FALSE)
                .setMinSampleShading(0.0f)
                .setAlphaToCoverageEnable(VK_FALSE)
                .setAlphaToOneEnable(VK_FALSE);

        out_state.depth_stencil.setDepthTestEnable(in_state.depth_test.enable)
                .setDepthWriteEnable(in_state.depth_test.write_enable)
                .setDepthBoundsTestEnable(in_state.depth_test.bound_test_enable)
                .setDepthCompareOp(_M_compare_funcs[static_cast<EnumerateType>(in_state.depth_test.func)])
                .setMinDepthBounds(in_state.depth_test.min_depth_bound)
                .setMaxDepthBounds(in_state.depth_test.max_depth_bound)
                .setStencilTestEnable(in_state.stencil_test.enable)
                .setFront(get_stencil_op_state(in_state.stencil_test.front))
                .setBack(get_stencil_op_state(in_state.stencil_test.back));

        out_state.color_blend_attachment.resize(in_state.color_blending.blend_attachment.size());

        Index index = 0;

        for (auto& attachment : out_state.color_blend_attachment)
        {
            attachment.setBlendEnable(in_state.color_blending.blend_attachment[index].enable)
                    .setSrcColorBlendFactor(_M_blend_factors[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].src_color_func)])
                    .setDstColorBlendFactor(_M_blend_factors[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].dst_color_func)])
                    .setColorBlendOp(_M_blend_ops[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].color_op)])
                    .setSrcAlphaBlendFactor(_M_blend_factors[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].src_alpha_func)])
                    .setDstAlphaBlendFactor(_M_blend_factors[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].dst_alpha_func)])
                    .setAlphaBlendOp(_M_blend_ops[static_cast<EnumerateType>(
                            in_state.color_blending.blend_attachment[index].alpha_op)]);

            vk::ColorComponentFlags color_mask;

            {
                ColorComponentMask R = mask_of<ColorComponentMask>(ColorComponent::R);
                ColorComponentMask G = mask_of<ColorComponentMask>(ColorComponent::G);
                ColorComponentMask B = mask_of<ColorComponentMask>(ColorComponent::B);
                ColorComponentMask A = mask_of<ColorComponentMask>(ColorComponent::A);

                if ((in_state.color_blending.blend_attachment[index].color_mask & R) == R)
                {
                    color_mask |= vk::ColorComponentFlagBits::eR;
                }
                if ((in_state.color_blending.blend_attachment[index].color_mask & G) == G)
                {
                    color_mask |= vk::ColorComponentFlagBits::eG;
                }
                if ((in_state.color_blending.blend_attachment[index].color_mask & B) == B)
                {
                    color_mask |= vk::ColorComponentFlagBits::eB;
                }
                if ((in_state.color_blending.blend_attachment[index].color_mask & A) == A)
                {
                    color_mask |= vk::ColorComponentFlagBits::eA;
                }

                attachment.setColorWriteMask(color_mask);
            }
            ++index;
        }

        out_state.color_blending.setBlendConstants(in_state.color_blending.blend_constants.array)
                .setAttachments(out_state.color_blend_attachment)
                .setLogicOpEnable(in_state.color_blending.logic_op_enable)
                .setLogicOp(_M_logic_ops[static_cast<EnumerateType>(in_state.color_blending.logic_op)]);
    }


    VulkanPipeline& VulkanPipeline::create(const Pipeline* pipeline)
    {
        destroy();
        create_descriptor_layout(pipeline);


        Vector<vk::PipelineShaderStageCreateInfo> pipeline_stage_create_infos;

        if (pipeline->vertex_shader)
        {
            pipeline_stage_create_infos.emplace_back(
                    vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex,
                    pipeline->vertex_shader->get_rhi_object<VulkanVertexShader>()->_M_shader, "main");
        }

        if (pipeline->fragment_shader)
        {
            pipeline_stage_create_infos.emplace_back(
                    vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,
                    pipeline->fragment_shader->get_rhi_object<VulkanFragmentShader>()->_M_shader, "main");
        }

        if (pipeline_stage_create_infos.empty())
        {
            throw EngineException("Failed to create pipeline without stages");
        }


        vk::PipelineVertexInputStateCreateInfo vertex_input_info;

        if (pipeline->vertex_shader)
        {
            vertex_input_info.setVertexBindingDescriptions(
                    pipeline->vertex_shader->get_rhi_object<VulkanVertexShader>()->_M_binding_description);
            vertex_input_info.setVertexAttributeDescriptions(
                    pipeline->vertex_shader->get_rhi_object<VulkanVertexShader>()->_M_attribute_description);
        }

        vk::Viewport viewport(0.f, 0.f, static_cast<float>(API->_M_swap_chain->_M_extent.width),
                              static_cast<float>(API->_M_swap_chain->_M_extent.height), 0.0f, 1.f);

        vk::Rect2D scissor({0, 0}, API->_M_swap_chain->_M_extent);

        vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

        vk::PipelineLayoutCreateInfo pipeline_layout_info({}, _M_descriptor_set_layout);

        _M_pipeline_layout = API->_M_device.createPipelineLayout(pipeline_layout_info);

        State out_state;
        init_pipeline_state(out_state, *pipeline);

        vk::GraphicsPipelineCreateInfo pipeline_info(
                {}, pipeline_stage_create_infos, &vertex_input_info, &out_state.input_assembly, nullptr,
                &viewport_state, &out_state.rasterizer, &out_state.multisampling, &out_state.depth_stencil,
                &out_state.color_blending, &dynamic_state_info, _M_pipeline_layout,
                pipeline->framebuffer->get_rhi_object<VulkanFramebuffer>()->_M_render_pass, 0, {});

        auto pipeline_result = API->_M_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
            API->_M_device.destroyPipeline(_M_pipeline);
            throw EngineException("Failed to create pipeline");
        }

        _M_pipeline = pipeline_result.value;

        return *this;
    }

    VulkanPipeline& VulkanPipeline::destroy()
    {
        DESTROY_CALL(destroyPipeline, _M_pipeline);
        return *this;
    }

    VulkanPipeline::~VulkanPipeline()
    {
        destroy();
    }

    void VulkanPipeline::bind()
    {
        if (API->_M_state->_M_pipeline != this)
        {
            API->_M_command_buffer->get().bindPipeline(vk::PipelineBindPoint::eGraphics, _M_pipeline);
            API->_M_state->_M_pipeline = this;
        }
    }

    RHI_Pipeline* VulkanAPI::create_pipeline(const Pipeline* pipeline)
    {
        return &(new VulkanPipeline())->create(pipeline);
    }
}// namespace Engine
