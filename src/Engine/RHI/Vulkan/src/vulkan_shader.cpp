#include <Graphics/shader.hpp>
#include <iostream>
#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_ssbo.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>
#include <vulkan_uniform_buffer.hpp>

#define SHADER_DATA info.binaries

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
    {}

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
                static_cast<VulkanFramebuffer*>(info.framebuffer)->_M_render_pass, 0, {});

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
            auto sizes = create_pool_size(info);
            for (VulkanDescriptorPool* instance : _M_descriptor_pool._M_instances)
            {
                instance->_M_pool_sizes = sizes;
            }
        }

        return true;
    }

    VulkanShader& VulkanShader::use()
    {
        if (API->_M_state->_M_shader != this)
        {
            API->_M_command_buffer->get().bindPipeline(vk::PipelineBindPoint::eGraphics, _M_pipeline);
            API->_M_state->_M_shader = this;
        }
        return *this;
    }

    VulkanShader& VulkanShader::bind_ubo(VulkanUniformBuffer* ubo, BindingIndex binding)
    {
        if (_M_descriptor_set_layout)
        {
            VulkanDescriptorSet* current_set = current_descriptor_set();

            if (current_set->_M_current_ubo[binding] != ubo)
            {
                vk::DescriptorBufferInfo buffer_info(ubo->_M_buffer, 0, ubo->_M_size);
                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eUniformBuffer, {}, buffer_info);
                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set->_M_current_ubo[binding] = ubo;
            }
        }

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
                                   MAX_BINDLESS_RESOURCES * info.uniform_buffers.size());
        }

        if (!info.texture_samplers.empty())
        {
            pool_size.emplace_back(vk::DescriptorType::eCombinedImageSampler,
                                   MAX_BINDLESS_RESOURCES * info.texture_samplers.size());
        }

        if (!info.shared_buffers.empty())
        {
            pool_size.emplace_back(vk::DescriptorType::eStorageBuffer,
                                   MAX_BINDLESS_RESOURCES * info.shared_buffers.size());
        }

        return pool_size;
    }

    VulkanDescriptorSet* VulkanShader::current_descriptor_set()
    {
        if (_M_last_frame != API->_M_current_frame)
        {
            _M_last_frame               = API->_M_current_frame;
            _M_current_descriptor_index = 0;
            _M_current_set              = nullptr;
        }

        auto& descriptor_array = _M_descriptor_sets.current();

        if (descriptor_array.size() <= _M_current_descriptor_index)
        {
            descriptor_array.push_back(_M_descriptor_pool.current().allocate_descriptor_set(_M_descriptor_set_layout));
        }

        VulkanDescriptorSet* new_set = descriptor_array[_M_current_descriptor_index];


        if (_M_current_set != new_set)
        {
            _M_current_set = &new_set->bind(_M_pipeline_layout);
        }

        return descriptor_array[_M_current_descriptor_index];
    }

    VulkanShader& VulkanShader::bind_texture_combined(struct VulkanSampler* sampler, VulkanTexture* texture,
                                                      uint_t binding)
    {
        if (_M_descriptor_set_layout)
        {
            VulkanDescriptorSet* current_set = current_descriptor_set();

            if (current_set->_M_sampler[binding] != sampler->_M_sampler ||
                current_set->_M_image_view[binding] != texture->_M_image_view)
            {
                vk::DescriptorImageInfo image_info(sampler->_M_sampler, texture->_M_image_view,
                                                   vk::ImageLayout::eShaderReadOnlyOptimal);

                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0,
                                                        vk::DescriptorType::eCombinedImageSampler, image_info);

                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set->_M_sampler[binding]    = sampler->_M_sampler;
                current_set->_M_image_view[binding] = texture->_M_image_view;
            }
        }
        return *this;
    }

    VulkanShader& VulkanShader::bind_sampler(struct VulkanSampler* sampler, BindingIndex location, BindingIndex binding)
    {
        if (_M_descriptor_set_layout)
        {
            VulkanDescriptorSet* current_set = current_descriptor_set();

            if (current_set->_M_sampler[binding] != sampler->_M_sampler)
            {
                vk::DescriptorImageInfo image_info(sampler->_M_sampler, {}, vk::ImageLayout::eShaderReadOnlyOptimal);
                vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0, vk::DescriptorType::eSampler,
                                                        image_info);

                API->_M_device.updateDescriptorSets(write_descriptor, {});
                current_set->_M_sampler[binding] = sampler->_M_sampler;
            }
        }
        return *this;
    }

    VulkanShader& VulkanShader::bind_shared_buffer(VulkanSSBO* ssbo, size_t offset, size_t size, uint_t binding)
    {
        if (_M_pipeline_layout && offset < ssbo->_M_size)
        {
            size                             = glm::min(size, ssbo->_M_size - offset);
            VulkanDescriptorSet* current_set = current_descriptor_set();
            vk::DescriptorBufferInfo buffer_info(ssbo->_M_buffer, offset, size);
            vk::WriteDescriptorSet write_descriptor(current_set->_M_set, binding, 0, vk::DescriptorType::eStorageBuffer,
                                                    {}, buffer_info);
            API->_M_device.updateDescriptorSets(write_descriptor, {});
        }
        return *this;
    }


    VulkanShader::~VulkanShader()
    {
        API->wait_idle();
        DESTROY_CALL(destroyDescriptorSetLayout, *_M_descriptor_set_layout);
        delete _M_descriptor_set_layout;
        _M_descriptor_set_layout = nullptr;

        DESTROY_CALL(destroyPipeline, _M_pipeline);
        DESTROY_CALL(destroyPipelineLayout, _M_pipeline_layout);
    }


    VulkanShaderBase& VulkanShaderBase::create(const ShaderBase* shader)
    {
        vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), shader->binary_code.size(),
                                        reinterpret_cast<const uint32_t*>(shader->binary_code.data()));
        _M_shader = API->_M_device.createShaderModule(info);

        return *this;
    }

    VulkanShaderBase& VulkanShaderBase::destroy()
    {
        DESTROY_CALL(destroyShaderModule, _M_shader)
        return *this;
    }


    VulkanVertexShader& VulkanVertexShader::create(const VertexShader* shader)
    {
        destroy();
        VulkanShaderBase::create(shader);

        _M_binding_description.reserve(shader->attributes.size());
        _M_attribute_description.reserve(shader->attributes.size());


        Index index = 0;
        for (auto& attribute : shader->attributes)
        {
            {
                _M_binding_description.emplace_back();
                vk::VertexInputBindingDescription& description = _M_binding_description.back();

                description.binding = static_cast<decltype(description.binding)>(index);
                description.stride  = attribute.type.size * attribute.type.count;

                switch (attribute.rate)
                {
                    case VertexAttributeInputRate::Instance:
                        description.inputRate = vk::VertexInputRate::eInstance;
                        break;

                    case VertexAttributeInputRate::Vertex:
                        description.inputRate = vk::VertexInputRate::eVertex;
                        break;

                    default:
                        throw EngineException("Undefined vertex attribute input rate!");
                }
            }

            {
                _M_binding_description.emplace_back();
                vk::VertexInputAttributeDescription& description = _M_attribute_description.back();
                description.binding                              = static_cast<decltype(description.binding)>(index);
                description.location                             = static_cast<decltype(description.location)>(index);
                description.offset                               = 0;// Each attribute has its own buffer
                description.format                               = get_type(attribute.type.type);
            }

            ++index;
        }

        return *this;
    }

    VulkanVertexShader& VulkanVertexShader::destroy()
    {
        VulkanShaderBase::destroy();
        _M_attribute_description.clear();
        _M_binding_description.clear();

        return *this;
    }

    VulkanVertexShader::~VulkanVertexShader()
    {
        destroy();
    }

    RHI_Shader* VulkanAPI::create_vertex_shader(const VertexShader* shader)
    {
        return &(new VulkanVertexShader())->create(shader);
    }

    RHI_Shader* VulkanAPI::create_fragment_shader(const FragmentShader* shader)
    {
        return nullptr;
    }
}// namespace Engine
