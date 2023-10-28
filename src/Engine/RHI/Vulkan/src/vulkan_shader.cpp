#include <Graphics/shader.hpp>
#include <iostream>
#include <thread>
#include <vulkan_api.hpp>
#include <vulkan_descriptor_pool.hpp>
#include <vulkan_descriptor_set.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

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

    VulkanShaderBase& VulkanShaderBase::create(const Shader* shader)
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
                description.stride  = Shader::stride_of(attribute.type);

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
                _M_attribute_description.emplace_back();
                vk::VertexInputAttributeDescription& description = _M_attribute_description.back();
                description.binding                              = static_cast<decltype(description.binding)>(index);
                description.location                             = static_cast<decltype(description.location)>(index);
                description.offset                               = 0;// Each attribute has its own buffer
                description.format =
                        parse_engine_format(static_cast<ColorFormat>(Shader::color_format_of(attribute.type)));
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

    VulkanFragmentShader& VulkanFragmentShader::create(const FragmentShader* shader)
    {
        destroy();
        VulkanShaderBase::create(shader);
        return *this;
    }

    VulkanFragmentShader& VulkanFragmentShader::destroy()
    {
        VulkanShaderBase::destroy();
        return *this;
    }

    VulkanFragmentShader::~VulkanFragmentShader()
    {
        destroy();
    }

    RHI_Shader* VulkanAPI::create_vertex_shader(const VertexShader* shader)
    {
        return &(new VulkanVertexShader())->create(shader);
    }

    RHI_Shader* VulkanAPI::create_fragment_shader(const FragmentShader* shader)
    {
        return &(new VulkanFragmentShader())->create(shader);
    }
}// namespace Engine
