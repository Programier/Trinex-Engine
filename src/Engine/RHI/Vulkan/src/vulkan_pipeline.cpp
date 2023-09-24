#include <Graphics/basic_framebuffer.hpp>
#include <Graphics/pipeline.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_definitions.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
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

    static void create_base_descriptor_layout(Shader* shader, Vector<Vector<vk::DescriptorSetLayoutBinding>>& out,
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

        for (auto& shared_buffer : shader->ssbo)
        {
            push_layout_binding(out, stage, shared_buffer.set, shared_buffer.binding,
                                vk::DescriptorType::eStorageBuffer);
        }
    }

    static void create_vertex_descriptor_layout(const Pipeline* pipeline,
                                                Vector<Vector<vk::DescriptorSetLayoutBinding>>& out)
    {
        create_base_descriptor_layout(pipeline->vertex_shader, out, vk::ShaderStageFlagBits::eVertex);
    }

    static void create_fragment_descriptor_layout(const Pipeline* pipeline,
                                                  Vector<Vector<vk::DescriptorSetLayoutBinding>>& out)
    {
        create_base_descriptor_layout(pipeline->fragment_shader, out, vk::ShaderStageFlagBits::eFragment);
    }


    VulkanPipeline& VulkanPipeline::create_descriptor_layout(const Pipeline* pipeline)
    {
        Vector<Vector<vk::DescriptorSetLayoutBinding>> layout_bindings;

        create_vertex_descriptor_layout(pipeline, layout_bindings);
        create_fragment_descriptor_layout(pipeline, layout_bindings);

        _M_descriptor_set_layout.resize(layout_bindings.size());

        Index set = 0;
        for (auto& layout : layout_bindings)
        {
            if (!layout_bindings.empty())
            {
                vk::DescriptorSetLayoutCreateInfo layout_info({}, layout);
                _M_descriptor_set_layout[set] = API->_M_device.createDescriptorSetLayout(layout_info);
            }
            ++set;
        }

        return *this;
    }

    VulkanDescriptorSet* VulkanPipeline::current_descriptor_set(BindingIndex set)
    {
        // I have no idea, what i writing :D
        if (_M_last_frame != API->_M_current_frame)
        {
            _M_last_frame = API->_M_current_frame;

            for (auto& binded_set : _M_binded_descriptor_sets)
            {
                binded_set._M_current_set              = nullptr;
                binded_set._M_current_descriptor_index = 0;
            }
        }

        auto& descriptor_array         = _M_descriptor_sets.current()[set];
        BindedDesriptorSet& binded_set = _M_binded_descriptor_sets[set];

        if (descriptor_array.size() <= binded_set._M_current_descriptor_index)
        {
            descriptor_array.push_back(
                    _M_descriptor_pool.current().allocate_descriptor_set(&_M_descriptor_set_layout[set], set));
        }

        VulkanDescriptorSet* new_set = descriptor_array[binded_set._M_current_descriptor_index];

        if (binded_set._M_current_set != new_set)
        {
            binded_set._M_current_set = &new_set->bind(_M_pipeline_layout, set);
        }

        return descriptor_array[binded_set._M_current_descriptor_index];
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
                    pipeline->vertex_shader->rhi_object<VulkanVertexShader>()->_M_shader, "main");
        }

        if (pipeline->fragment_shader)
        {
            pipeline_stage_create_infos.emplace_back(
                    vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment,
                    pipeline->fragment_shader->rhi_object<VulkanFragmentShader>()->_M_shader, "main");
        }

        if (pipeline_stage_create_infos.empty())
        {
            throw EngineException("Failed to create pipeline without stages");
        }


        vk::PipelineVertexInputStateCreateInfo vertex_input_info;

        if (pipeline->vertex_shader)
        {
            vertex_input_info.setVertexBindingDescriptions(
                    pipeline->vertex_shader->rhi_object<VulkanVertexShader>()->_M_binding_description);
            vertex_input_info.setVertexAttributeDescriptions(
                    pipeline->vertex_shader->rhi_object<VulkanVertexShader>()->_M_attribute_description);
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
                pipeline->render_target->render_pass->rhi_object<VulkanRenderPass>()->_M_render_pass, 0, {});

        auto pipeline_result = API->_M_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
            API->_M_device.destroyPipeline(_M_pipeline);
            throw EngineException("Failed to create pipeline");
        }

        _M_pipeline = pipeline_result.value;

        if (!_M_descriptor_set_layout.empty())
        {
            auto sizes = create_pool_sizes(pipeline);
            for (VulkanDescriptorPool* instance : _M_descriptor_pool._M_instances)
            {
                instance->pool_sizes(sizes);
            }

            for (auto& descriptor_set : _M_descriptor_sets._M_instances)
            {
                descriptor_set->resize(sizes.size());
            }

            _M_binded_descriptor_sets.resize(sizes.size());
        }

        return *this;
    }

    VulkanPipeline& VulkanPipeline::destroy()
    {
        DESTROY_CALL(destroyPipeline, _M_pipeline);
        return *this;
    }

    struct PoolSizeInfo {
        uint_t samplers;
        uint_t combined_samplers;
        uint_t textures;
        uint_t ubos;
        uint_t ssbos;
    };

    static void process_pool_sizes(Shader* shader, Vector<PoolSizeInfo>& out)
    {
        if (!shader)
            return;
    }

    Vector<Vector<vk::DescriptorPoolSize>> VulkanPipeline::create_pool_sizes(const Pipeline* pipeline)
    {
        Vector<PoolSizeInfo> pool_size_info(_M_descriptor_set_layout.size(), PoolSizeInfo{});

        process_pool_sizes(pipeline->vertex_shader, pool_size_info);
        process_pool_sizes(pipeline->fragment_shader, pool_size_info);


        Vector<Vector<vk::DescriptorPoolSize>> pool_sizes(pool_size_info.size());

        Index index = 0;

        for (auto& pool_size : pool_sizes)
        {
            PoolSizeInfo& info = pool_size_info[index];

            if (info.samplers)
                pool_size.push_back(
                        vk::DescriptorPoolSize(vk::DescriptorType::eSampler, MAX_BINDLESS_RESOURCES * info.samplers));
            if (info.combined_samplers)
                pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                                                           MAX_BINDLESS_RESOURCES * info.combined_samplers));
            if (info.textures)
                pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage,
                                                           MAX_BINDLESS_RESOURCES * info.textures));
            if (info.ubos)
                pool_size.push_back(
                        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_BINDLESS_RESOURCES * info.ubos));
            if (info.ssbos)
                pool_size.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer,
                                                           MAX_BINDLESS_RESOURCES * info.ssbos));

            ++index;
        }

        return pool_sizes;
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

    VulkanPipeline& VulkanPipeline::bind_ssbo(struct VulkanSSBO* ssbo, BindingIndex binding, BindingIndex set)
    {
        if (!_M_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(set);

            VulkanSSBO*& current_ssbo = current_set->_M_ssbo[binding];
            if (current_ssbo != ssbo)
            {
                vk::DescriptorBufferInfo buffer_info(ssbo->_M_buffer._M_buffer, 0, ssbo->_M_buffer._M_size);
                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eStorageBuffer, {}, buffer_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_ssbo = ssbo;
            }
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_uniform_buffer(VulkanUniformBuffer* buffer, BindingIndex binding,
                                                        BindingIndex set)
    {
        if (!_M_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(set);

            if (current_set->_M_current_ubo[binding] != buffer)
            {
                vk::DescriptorBufferInfo buffer_info(buffer->current_buffer()->_M_buffer, 0,
                                                     buffer->current_buffer()->_M_size);

                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eUniformBuffer, {}, buffer_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set->_M_current_ubo[binding] = buffer;
            }
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_combined_sampler(struct VulkanSampler* sampler, struct VulkanTexture* texture,
                                                          BindingIndex binding, BindingIndex set)
    {
        if (!_M_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(set);

            VulkanDescriptorSet::CombinedImageSampler& info = current_set->_M_combined_image_sampler[binding];
            if (info._M_sampler != sampler || info._M_texture != texture)
            {
                vk::DescriptorImageInfo image_info(sampler->_M_sampler, texture->_M_image_view,
                                                   vk::ImageLayout::eShaderReadOnlyOptimal);

                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eCombinedImageSampler, image_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                info._M_sampler = sampler;
                info._M_texture = texture;
            }
        }

        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_sampler(VulkanSampler* sampler, BindingIndex binding, BindingIndex set)
    {
        if (!_M_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(set);

            VulkanSampler*& current_sampler = current_set->_M_sampler[binding];
            if (current_sampler != sampler)
            {
                vk::DescriptorImageInfo image_info(sampler->_M_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0, vk::DescriptorType::eSampler,
                                                        image_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_sampler = sampler;
            }
        }
        return *this;
    }

    VulkanPipeline& VulkanPipeline::bind_texture(VulkanTexture* texture, BindingIndex binding, BindingIndex set)
    {
        if (!_M_descriptor_set_layout.empty())
        {
            VulkanDescriptorSet* current_set = current_descriptor_set(set);

            VulkanTexture*& current_texture = current_set->_M_texture[binding];
            if (current_texture != texture)
            {
                vk::DescriptorImageInfo image_info({}, texture->_M_image_view, vk::ImageLayout::eShaderReadOnlyOptimal);
                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eSampledImage, image_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_texture = texture;
            }
        }

        return *this;
    }

    // Must be called after draw call
    VulkanPipeline& VulkanPipeline::increment_set_index()
    {
        for (auto& set : _M_binded_descriptor_sets)
        {
            ++set._M_current_descriptor_index;
        }
        return *this;
    }

    RHI_Pipeline* VulkanAPI::create_pipeline(const Pipeline* pipeline)
    {
        return &(new VulkanPipeline())->create(pipeline);
    }
}// namespace Engine
