#include <Graphics/shader.hpp>
#include <vulkan_api.hpp>
#include <vulkan_sampler.hpp>
#include <vulkan_shader.hpp>
#include <vulkan_state.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_types.hpp>

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

	VulkanShader::VulkanShader(const byte* shader, size_t size)
	{
		vk::ShaderModuleCreateInfo info(vk::ShaderModuleCreateFlags(), size, reinterpret_cast<const uint32_t*>(shader));
		m_shader = API->m_device.createShaderModule(info);
	}

	VulkanShader::~VulkanShader()
	{
		DESTROY_CALL(destroyShaderModule, m_shader)
	}

	static vk::Format parse_vertex_format(VertexBufferElementType type, uint32_t& stride)
	{
		switch (type)
		{
			case VertexBufferElementType::Float1: stride = sizeof(float); return vk::Format::eR32Sfloat;
			case VertexBufferElementType::Float2: stride = sizeof(float) * 2; return vk::Format::eR32G32Sfloat;
			case VertexBufferElementType::Float3: stride = sizeof(float) * 3; return vk::Format::eR32G32B32Sfloat;
			case VertexBufferElementType::Float4: stride = sizeof(float) * 4; return vk::Format::eR32G32B32A32Sfloat;
			case VertexBufferElementType::Byte1: stride = sizeof(signed_byte); return vk::Format::eR8Sint;
			case VertexBufferElementType::Byte2: stride = sizeof(signed_byte) * 2; return vk::Format::eR8G8Sint;
			case VertexBufferElementType::Byte4: stride = sizeof(signed_byte) * 4; return vk::Format::eR8G8B8A8Sint;
			case VertexBufferElementType::Byte1N: stride = sizeof(signed_byte); return vk::Format::eR8Snorm;
			case VertexBufferElementType::Byte2N: stride = sizeof(signed_byte) * 2; return vk::Format::eR8G8Snorm;
			case VertexBufferElementType::Byte4N: stride = sizeof(signed_byte) * 4; return vk::Format::eR8G8B8A8Snorm;
			case VertexBufferElementType::UByte1: stride = sizeof(byte); return vk::Format::eR8Uint;
			case VertexBufferElementType::UByte2: stride = sizeof(byte) * 2; return vk::Format::eR8G8Uint;
			case VertexBufferElementType::UByte4: stride = sizeof(byte) * 4; return vk::Format::eR8G8B8A8Uint;
			case VertexBufferElementType::UByte1N: stride = sizeof(byte); return vk::Format::eR8Unorm;
			case VertexBufferElementType::UByte2N: stride = sizeof(byte) * 2; return vk::Format::eR8G8Unorm;
			case VertexBufferElementType::UByte4N: stride = sizeof(byte) * 4; return vk::Format::eR8G8B8A8Unorm;
			case VertexBufferElementType::Short1: stride = sizeof(short_t); return vk::Format::eR16Sint;
			case VertexBufferElementType::Short2: stride = sizeof(short_t) * 2; return vk::Format::eR16G16Sint;
			case VertexBufferElementType::Short4: stride = sizeof(short_t) * 4; return vk::Format::eR16G16B16A16Sint;
			case VertexBufferElementType::Short1N: stride = sizeof(short_t); return vk::Format::eR16Snorm;
			case VertexBufferElementType::Short2N: stride = sizeof(short_t) * 2; return vk::Format::eR16G16Snorm;
			case VertexBufferElementType::Short4N: stride = sizeof(short_t) * 4; return vk::Format::eR16G16B16A16Snorm;
			case VertexBufferElementType::UShort1: stride = sizeof(uint16_t); return vk::Format::eR16Uint;
			case VertexBufferElementType::UShort2: stride = sizeof(uint16_t) * 2; return vk::Format::eR16G16Uint;
			case VertexBufferElementType::UShort4: stride = sizeof(uint16_t) * 4; return vk::Format::eR16G16B16A16Uint;
			case VertexBufferElementType::UShort1N: stride = sizeof(uint16_t); return vk::Format::eR16Unorm;
			case VertexBufferElementType::UShort2N: stride = sizeof(uint16_t) * 2; return vk::Format::eR16G16Unorm;
			case VertexBufferElementType::UShort4N: stride = sizeof(uint16_t) * 4; return vk::Format::eR16G16B16A16Unorm;
			case VertexBufferElementType::Int1: stride = sizeof(int32_t); return vk::Format::eR32Sint;
			case VertexBufferElementType::Int2: stride = sizeof(int32_t) * 2; return vk::Format::eR32G32Sint;
			case VertexBufferElementType::Int3: stride = sizeof(int32_t) * 3; return vk::Format::eR32G32B32Sint;
			case VertexBufferElementType::Int4: stride = sizeof(int32_t) * 4; return vk::Format::eR32G32B32A32Sint;
			case VertexBufferElementType::UInt1: stride = sizeof(uint32_t); return vk::Format::eR32Uint;
			case VertexBufferElementType::UInt2: stride = sizeof(uint32_t) * 2; return vk::Format::eR32G32Uint;
			case VertexBufferElementType::UInt3: stride = sizeof(uint32_t) * 3; return vk::Format::eR32G32B32Uint;
			case VertexBufferElementType::UInt4: stride = sizeof(uint32_t) * 4; return vk::Format::eR32G32B32A32Uint;
			default: return vk::Format::eUndefined;
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

	VulkanVertexShader::VulkanVertexShader(const byte* shader, size_t size, const VertexAttribute* attributes,
	                                       size_t attributes_count)
	    : VulkanShader(shader, size)
	{
		m_binding_description.reserve(attributes_count);
		m_attribute_description.reserve(attributes_count);

		for (size_t index = 0; index < attributes_count; ++index)
		{
			auto& attribute   = attributes[index];
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
		}
	}


	RHI_Shader* VulkanAPI::create_vertex_shader(const byte* shader, size_t size, const VertexAttribute* attributes,
	                                            size_t attributes_count)
	{
		return new VulkanVertexShader(shader, size, attributes, attributes_count);
	}

	RHI_Shader* VulkanAPI::create_tesselation_control_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}

	RHI_Shader* VulkanAPI::create_tesselation_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}

	RHI_Shader* VulkanAPI::create_geometry_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}

	RHI_Shader* VulkanAPI::create_fragment_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}

	RHI_Shader* VulkanAPI::create_compute_shader(const byte* shader, size_t size)
	{
		return new VulkanShader(shader, size);
	}
}// namespace Engine
