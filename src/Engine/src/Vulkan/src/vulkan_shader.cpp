#include <iostream>
#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_async_command_buffer.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_ssbo.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>
#include <vulkan_uniform_buffer.hpp>


#define SHADER_DATA info.binaries
#define CURRENT_UBO _M_descriptor_sets[API->_M_current_frame]

namespace Engine
{

    static Vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    static vk::PipelineDynamicStateCreateInfo dynamic_state_info({}, dynamic_states);


    struct VulkanPipelineState {
        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        vk::PipelineRasterizationStateCreateInfo rasterizer;
        vk::PipelineMultisampleStateCreateInfo multisampling;
        vk::PipelineDepthStencilStateCreateInfo depth_stencil;
        Vector<vk::PipelineColorBlendAttachmentState> color_blend_attachment;
        vk::PipelineColorBlendStateCreateInfo color_blending;
        vk::SampleMask sample_mask;
    };

    // Use template, becouse PipelineState uses no-name structures
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

    static void init_pipeline_state(VulkanPipelineState& out_state, const PipelineCreateInfo& info)
    {
        const PipelineState& in_state                   = info.state;
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

            if ((in_state.color_blending.blend_attachment[index].color_mask & ColorComponent::R) == ColorComponent::R)
            {
                color_mask |= vk::ColorComponentFlagBits::eR;
            }
            if ((in_state.color_blending.blend_attachment[index].color_mask & ColorComponent::G) == ColorComponent::G)
            {
                color_mask |= vk::ColorComponentFlagBits::eG;
            }
            if ((in_state.color_blending.blend_attachment[index].color_mask & ColorComponent::B) == ColorComponent::B)
            {
                color_mask |= vk::ColorComponentFlagBits::eB;
            }
            if ((in_state.color_blending.blend_attachment[index].color_mask & ColorComponent::A) == ColorComponent::A)
            {
                color_mask |= vk::ColorComponentFlagBits::eA;
            }

            attachment.setColorWriteMask(color_mask);
            ++index;
        }

        out_state.color_blending.setBlendConstants(in_state.color_blending.blend_constants.array)
                .setAttachments(out_state.color_blend_attachment)
                .setLogicOpEnable(in_state.color_blending.logic_op_enable)
                .setLogicOp(_M_logic_ops[static_cast<EnumerateType>(in_state.color_blending.logic_op)]);
    }


    static void create_shader_module(const FileBuffer& buffer, Vector<vk::ShaderModule*>& modules)
    {
        if (buffer.empty())
        {
            modules.push_back(nullptr);
            return;
        }

        vk::ShaderModuleCreateInfo create_info(vk::ShaderModuleCreateFlags(), buffer.size(),
                                               reinterpret_cast<const uint32_t*>(buffer.data()));
        modules.push_back(new vk::ShaderModule(API->_M_device.createShaderModule(create_info)));
    }


    static vk::ShaderStageFlagBits get_stage_by_index(std::size_t index)
    {
        static const vk::ShaderStageFlagBits flags[] = {
                vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment, vk::ShaderStageFlagBits::eCompute,
                vk::ShaderStageFlagBits::eGeometry};

        if (index > 3)
            throw std::runtime_error("Undefined shader stage index");
        return flags[index];
    }

    VulkanShader::VulkanShader()
    {
        _M_instance_address = this;
    }

    VulkanShader& VulkanShader::create_descriptor_layout(const PipelineCreateInfo& info)
    {
        size_t count = info.uniform_buffers.size() + info.texture_samplers.size() + info.shared_buffers.size();
        if (count == 0)
            return *this;

        Vector<vk::DescriptorSetLayoutBinding> layout_bindings(count);

        ArrayIndex index = 0;

        for (auto& buffer : info.uniform_buffers)
        {
            layout_bindings[index++] = vk::DescriptorSetLayoutBinding(
                    buffer.binding, vk::DescriptorType::eUniformBuffer, 1,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr);
        }

        for (auto& texture_sampler : info.texture_samplers)
        {
            layout_bindings[index++] =
                    vk::DescriptorSetLayoutBinding(texture_sampler.binding, vk::DescriptorType::eCombinedImageSampler,
                                                   1, vk::ShaderStageFlagBits::eFragment, nullptr);
        }

        for (auto& shared_buffer : info.shared_buffers)
        {
            layout_bindings[index++] = vk::DescriptorSetLayoutBinding(
                    shared_buffer.binding, vk::DescriptorType::eStorageBuffer, 1,
                    vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex, nullptr);
        }

        vk::DescriptorSetLayoutCreateInfo layout_info({}, layout_bindings);
        _M_descriptor_set_layout = new vk::DescriptorSetLayout(API->_M_device.createDescriptorSetLayout(layout_info));
        return *this;
    }


    bool VulkanShader::init(const PipelineCreateInfo& info)
    {
        _M_has_descriptors      = false;
        _M_max_descriptors_sets = info.max_textures_binding_per_frame;
        create_descriptor_layout(info);


        Vector<vk::ShaderModule*> shader_modules;
        create_shader_module(info.binaries.vertex, shader_modules);
        create_shader_module(info.binaries.fragment, shader_modules);
        create_shader_module(info.binaries.compute, shader_modules);
        create_shader_module(info.binaries.geometry, shader_modules);

        Vector<vk::PipelineShaderStageCreateInfo> pipeline_shader_stage_create_infos;

        std::size_t index = 0;
        for (auto module : shader_modules)
        {
            if (module)
            {
                pipeline_shader_stage_create_infos.emplace_back(vk::PipelineShaderStageCreateFlags(),
                                                                get_stage_by_index(index), *module, "main");
            }
            ++index;
        }

        if (pipeline_shader_stage_create_infos.empty())
        {
            std::runtime_error("Failed to create Shader");
            return false;
        }


        auto binding_desc = get_binding_description(info);
        auto attrib_desc  = get_attribute_description(info);


        vk::PipelineVertexInputStateCreateInfo vertex_input_info({}, binding_desc.size(), binding_desc.data(),
                                                                 attrib_desc.size(),
                                                                 attrib_desc.empty() ? nullptr : attrib_desc.data());

        vk::Viewport viewport(0.f, 0.f, static_cast<float>(API->_M_swap_chain->_M_extent.width),
                              static_cast<float>(API->_M_swap_chain->_M_extent.height), 0.0f, 1.f);

        vk::Rect2D scissor({0, 0}, API->_M_swap_chain->_M_extent);

        vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);

        vk::PipelineLayoutCreateInfo pipeline_layout_info({}, _M_descriptor_set_layout ? 1 : 0,
                                                          _M_descriptor_set_layout);

        _M_pipeline_layout = API->_M_device.createPipelineLayout(pipeline_layout_info);

        VulkanPipelineState out_state;
        init_pipeline_state(out_state, info);

        vk::GraphicsPipelineCreateInfo pipeline_info(
                {}, pipeline_shader_stage_create_infos, &vertex_input_info, &out_state.input_assembly, nullptr,
                &viewport_state, &out_state.rasterizer, &out_state.multisampling, &out_state.depth_stencil,
                &out_state.color_blending, &dynamic_state_info, _M_pipeline_layout,
                API->framebuffer(info.framebuffer_usage)->_M_render_pass, 0, {});

        auto pipeline_result = API->_M_device.createGraphicsPipeline({}, pipeline_info);

        if (pipeline_result.result != vk::Result::eSuccess)
        {
            API->_M_device.destroyPipeline(_M_pipeline);
            throw std::runtime_error("Failed to create pipeline");
            return false;
        }

        _M_pipeline = pipeline_result.value;

        for (auto module : shader_modules)
        {
            if (module)
            {
                API->_M_device.destroyShaderModule(*module);
                delete module;
            }
        }

        if (_M_descriptor_set_layout)
        {
            Vector<vk::DescriptorPoolSize> pool_size = create_pool_size(info);
            vk::DescriptorPoolCreateInfo pool_info({}, MAIN_FRAMEBUFFERS_COUNT * _M_max_descriptors_sets, pool_size);
            _M_descriptor_pool = API->_M_device.createDescriptorPool(pool_info);
            create_descriptor_sets();
        }

        return true;
    }

    VulkanShader& VulkanShader::update_descriptor_layout(bool force)
    {
        if (_M_current_binded_descriptor_index != _M_current_descriptor_index || force)
        {
            _M_current_binded_descriptor_index = _M_current_descriptor_index;
            auto& current_set                  = current_descriptor_set();

            API->_M_current_command_buffer->get()->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                                      _M_pipeline_layout, 0, 1,
                                                                      &current_set._M_descriptor_set, 0, nullptr);
        }
        return *this;
    }

    VulkanShader& VulkanShader::use()
    {
        API->_M_current_command_buffer->get()->bindPipeline(vk::PipelineBindPoint::eGraphics, _M_pipeline);
        API->_M_current_command_buffer->get_threaded_command_buffer()->_M_current_shader = this;
        update_descriptor_layout(true);
        return *this;
    }


    VulkanShader& VulkanShader::bind_ubo(VulkanUniformBuffer* ubo, BindingIndex binding)
    {
        if (_M_has_descriptors)
        {
            auto& current_set = current_descriptor_set();
            update_descriptor_layout();

            if (current_set._M_current_ubo[binding] != ubo)
            {
                vk::DescriptorBufferInfo buffer_info(ubo->_M_block._M_block->_M_buffer, ubo->_M_block._M_offset,
                                                     ubo->_M_block._M_size);
                vk::WriteDescriptorSet write_descriptor(current_set._M_descriptor_set, binding, 0,
                                                        vk::DescriptorType::eUniformBuffer, {}, buffer_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set._M_current_ubo[binding] = ubo;
            }
        }

        return *this;
    }

    VulkanShader& VulkanShader::create_new_descriptor_set(uint32_t buffer_index)
    {
        auto& descriptor_array = _M_descriptor_sets[buffer_index];
        if (descriptor_array.size() == static_cast<size_t>(_M_max_descriptors_sets))
        {
            throw std::runtime_error("Vulkan API: Failed to create descriptor set. Count of sets is out of range!");
        }

        vk::DescriptorSetAllocateInfo alloc_info(_M_descriptor_pool, *_M_descriptor_set_layout);
        auto sets = API->_M_device.allocateDescriptorSets(alloc_info);


        descriptor_array.emplace_back(sets.front());
        return *this;
    }

    VulkanShader& VulkanShader::create_descriptor_sets()
    {
        _M_descriptor_sets.resize(MAIN_FRAMEBUFFERS_COUNT);
        for (Counter i = 0; i < MAIN_FRAMEBUFFERS_COUNT; i++) create_new_descriptor_set(i);
        _M_has_descriptors = true;
        return *this;
    }

    Vector<vk::VertexInputBindingDescription> VulkanShader::get_binding_description(const PipelineCreateInfo& info)
    {
        if (info.vertex_info.attributes.empty())
            return {};

        return {vk::VertexInputBindingDescription(0, info.vertex_info.size, vk::VertexInputRate::eVertex)};
    }

    Vector<vk::VertexInputAttributeDescription> VulkanShader::get_attribute_description(const PipelineCreateInfo& info)
    {
        if (info.vertex_info.attributes.empty())
            return {};

        Vector<vk::VertexInputAttributeDescription> attribute_descriptions(info.vertex_info.attributes.size());

        ArrayIndex index = 0;

        for (auto& attribute : info.vertex_info.attributes)
        {
            attribute_descriptions[index] = vk::VertexInputAttributeDescription(
                    index, 0, _M_shader_data_types[static_cast<uint_t>(attribute.type.type)], attribute.offset);
            ++index;
        }

        return attribute_descriptions;
    }

    Vector<vk::DescriptorPoolSize> VulkanShader::create_pool_size(const PipelineCreateInfo& info)
    {
        Vector<vk::DescriptorPoolSize> pool_size;

        if (!info.uniform_buffers.empty())
        {
            pool_size.emplace_back(vk::DescriptorType::eUniformBuffer,
                                   MAIN_FRAMEBUFFERS_COUNT * info.uniform_buffers.size());
        }

        if (!info.texture_samplers.empty())
        {
            pool_size.emplace_back(vk::DescriptorType::eCombinedImageSampler,
                                   MAIN_FRAMEBUFFERS_COUNT * info.texture_samplers.size());
        }

        return pool_size;
    }

    DescriptorSetInfo& VulkanShader::current_descriptor_set()
    {
        if (_M_last_frame != API->_M_current_frame)
        {
            _M_last_frame               = API->_M_current_frame;
            _M_current_descriptor_index = 0;
        }

        auto& descriptor_array = _M_descriptor_sets[API->_M_current_frame];


        if (descriptor_array.size() <= _M_current_descriptor_index)
        {
            create_new_descriptor_set(API->_M_current_frame);
        }
        return descriptor_array[_M_current_descriptor_index];
    }

    VulkanShader& VulkanShader::bind_texture(VulkanTexture* texture, uint_t binding)
    {
        if (_M_has_descriptors)
        {
            auto& current_set = current_descriptor_set();
            update_descriptor_layout();

            if (current_set._M_sampler != texture->_M_texture_sampler ||
                current_set._M_image_view != texture->_M_image_view)
            {
                vk::DescriptorImageInfo image_info(texture->_M_texture_sampler, texture->_M_image_view,
                                                   vk::ImageLayout::eShaderReadOnlyOptimal);

                vk::WriteDescriptorSet write_descriptor(current_set._M_descriptor_set, binding, 0,
                                                        vk::DescriptorType::eCombinedImageSampler, image_info);

                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set._M_sampler    = texture->_M_texture_sampler;
                current_set._M_image_view = texture->_M_image_view;
            }
        }
        return *this;
    }

    VulkanShader& VulkanShader::bind_shared_buffer(VulkanSSBO* ssbo, size_t offset, size_t size, uint_t binding)
    {
        if (_M_has_descriptors && offset < ssbo->_M_size)
        {
            size              = glm::min(size, ssbo->_M_size - offset);
            auto& current_set = current_descriptor_set();
            update_descriptor_layout();

            vk::DescriptorBufferInfo buffer_info(ssbo->_M_buffer, offset, size);

            vk::WriteDescriptorSet write_descriptor(current_set._M_descriptor_set, binding, 0,
                                                    vk::DescriptorType::eStorageBuffer, {}, buffer_info);
            API->_M_device.updateDescriptorSets(write_descriptor, {});
        }
        return *this;
    }


    VulkanShader::~VulkanShader()
    {
        API->wait_idle();
        DESTROY_CALL(destroyDescriptorPool, _M_descriptor_pool);

        _M_descriptor_sets.clear();

        DESTROY_CALL(destroyDescriptorSetLayout, *_M_descriptor_set_layout);
        delete _M_descriptor_set_layout;
        _M_descriptor_set_layout = nullptr;

        DESTROY_CALL(destroyPipeline, _M_pipeline);
        DESTROY_CALL(destroyPipelineLayout, _M_pipeline_layout);
    }

}// namespace Engine
