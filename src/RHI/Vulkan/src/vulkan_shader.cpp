#include <Graphics/shader.hpp>
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
        if (shader->source_code.empty())
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

    static vk::Format parse_vertex_format(VertexBufferElementType type, uint32_t& stride)
    {
        switch (type)
        {
            case VertexBufferElementType::Float1:
                stride = sizeof(float);
                return vk::Format::eR32Sfloat;

            case VertexBufferElementType::Float2:
                stride = sizeof(float) * 2;
                return vk::Format::eR32G32Sfloat;

            case VertexBufferElementType::Float3:
                stride = sizeof(float) * 3;
                return vk::Format::eR32G32B32Sfloat;

            case VertexBufferElementType::Float4:
                stride = sizeof(float) * 4;
                return vk::Format::eR32G32B32A32Sfloat;

            case VertexBufferElementType::UByte4:
                stride = sizeof(byte) * 4;
                return vk::Format::eR8G8B8A8Uint;

            case VertexBufferElementType::UByte4N:
                stride = sizeof(byte) * 4;
                return vk::Format::eR8G8B8A8Unorm;

            case VertexBufferElementType::Color:
                stride = sizeof(byte) * 4;
                return vk::Format::eR8G8B8A8Unorm;
            default:
                return vk::Format::eUndefined;
        }
    }

    static vk::VertexInputRate rate_of(VertexAttributeInputRate rate)
    {
        return rate == VertexAttributeInputRate::Vertex ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance;
    }

    static const char* name_of_rate(vk::VertexInputRate rate)
    {
        return rate == vk::VertexInputRate::eVertex ? "Vertex" : "Instance";
    }

    bool VulkanVertexShader::create(const VertexShader* shader)
    {
        destroy();
        bool status = VulkanShaderBase::create(shader);
        if (status == false)
            return false;

        m_binding_description.reserve(shader->attributes.size());
        m_attribute_description.reserve(shader->attributes.size());

        for (Index index = 0; auto& attribute : shader->attributes)
        {
            uint32_t stream   = static_cast<uint32_t>(attribute.stream_index);
            uint32_t stride   = 0;
            vk::Format format = parse_vertex_format(attribute.type, stride);

            {
                // Find and setup binding description
                bool found = false;
                for (auto& desc : m_binding_description)
                {
                    if (desc.binding == stream)
                    {
                        vk::VertexInputRate rate = rate_of(attribute.rate);

                        if (desc.inputRate != rate)
                        {
                            error_log("Vulkan", "Stream '%d' already used for '%s' rate, but attribute '%zu' has rate '%s'",
                                      name_of_rate(rate), index, name_of_rate(desc.inputRate));
                            return false;
                        }

                        desc.stride = glm::max(desc.stride, stride + static_cast<uint32_t>(attribute.offset));
                        found       = true;
                        break;
                    }
                }

                if (!found)
                {
                    m_binding_description.emplace_back();
                    auto& desc     = m_binding_description.back();
                    desc.binding   = attribute.stream_index;
                    desc.stride    = stride;
                    desc.inputRate = rate_of(attribute.rate);
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
        if (out->create(in))
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
