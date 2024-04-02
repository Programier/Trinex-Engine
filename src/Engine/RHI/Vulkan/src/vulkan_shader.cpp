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

    bool VulkanShaderBase::create(const Shader* shader)
    {
        if(shader->source_code.empty())
            return false;
        vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), shader->source_code.size(),
                                        reinterpret_cast<const uint32_t*>(shader->source_code.data()));
        m_shader = API->m_device.createShaderModule(info);

        return true;
    }

    VulkanShaderBase& VulkanShaderBase::destroy()
    {
        DESTROY_CALL(destroyShaderModule, m_shader)
        return *this;
    }


    bool VulkanVertexShader::create(const VertexShader* shader)
    {
        destroy();
        bool status = VulkanShaderBase::create(shader);
        if(status == false)
            return false;

        m_binding_description.reserve(shader->attributes.size());
        m_attribute_description.reserve(shader->attributes.size());


        Index index = 0;
        for (auto& attribute : shader->attributes)
        {
            {
                m_binding_description.emplace_back();
                vk::VertexInputBindingDescription& description = m_binding_description.back();
                ColorFormatInfo format_info                    = ColorFormatInfo::info_of(attribute.format);

                description.binding = static_cast<decltype(description.binding)>(index);
                description.stride  = format_info.size() * static_cast<uint_t>(attribute.count);

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
                m_attribute_description.emplace_back();
                vk::VertexInputAttributeDescription& description = m_attribute_description.back();
                description.binding                              = static_cast<decltype(description.binding)>(index);
                description.location                             = static_cast<decltype(description.location)>(index);
                description.offset                               = 0;// Each attribute has its own buffer
                description.format                               = parse_engine_format(attribute.format);
            }

            ++index;
        }

        return status;
    }

    VulkanVertexShader& VulkanVertexShader::destroy()
    {
        VulkanShaderBase::destroy();
        m_attribute_description.clear();
        m_binding_description.clear();

        return *this;
    }

    VulkanVertexShader::~VulkanVertexShader()
    {
        destroy();
    }

    bool VulkanFragmentShader::create(const FragmentShader* shader)
    {
        destroy();
        return VulkanShaderBase::create(shader);
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


    bool VulkanTessellationControlShader::create(const TessellationControlShader* shader)
    {
        destroy();
        return VulkanShaderBase::create(shader);
    }

    VulkanTessellationControlShader& VulkanTessellationControlShader::destroy()
    {
        VulkanShaderBase::destroy();
        return *this;
    }

    VulkanTessellationControlShader::~VulkanTessellationControlShader()
    {
        destroy();
    }

    bool VulkanTessellationShader::create(const TessellationShader* shader)
    {
        destroy();
        return VulkanShaderBase::create(shader);
    }

    VulkanTessellationShader& VulkanTessellationShader::destroy()
    {
        VulkanShaderBase::destroy();
        return *this;
    }

    VulkanTessellationShader::~VulkanTessellationShader()
    {
        destroy();
    }

    bool VulkanGeometryShader::create(const GeometryShader* shader)
    {
        destroy();
        return VulkanShaderBase::create(shader);
    }

    VulkanGeometryShader& VulkanGeometryShader::destroy()
    {
        VulkanShaderBase::destroy();
        return *this;
    }

    VulkanGeometryShader::~VulkanGeometryShader()
    {
        destroy();
    }


    template<typename Out, typename In>
    static Out* create_shader(In* in)
    {
        Out* out = new Out();
        if(out->create(in))
        {
            return out;
        }

        delete out;
        return nullptr;
    }

    RHI_Shader* VulkanAPI::create_vertex_shader(const VertexShader* shader)
    {
        return create_shader<VulkanVertexShader>(shader);
    }

    RHI_Shader* VulkanAPI::create_tesselation_control_shader(const TessellationControlShader* shader)
    {
        return create_shader<VulkanTessellationControlShader>(shader);
    }

    RHI_Shader* VulkanAPI::create_tesselation_shader(const TessellationShader* shader)
    {
        return create_shader<VulkanTessellationShader>(shader);
    }

    RHI_Shader* VulkanAPI::create_geometry_shader(const GeometryShader* shader)
    {
        return create_shader<VulkanGeometryShader>(shader);
    }

    RHI_Shader* VulkanAPI::create_fragment_shader(const FragmentShader* shader)
    {
        return create_shader<VulkanFragmentShader>(shader);
    }
}// namespace Engine
