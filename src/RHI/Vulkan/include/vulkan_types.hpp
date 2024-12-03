#pragma once
#include <Core/enums.hpp>
#include <Core/etl/array.hpp>
#include <stdexcept>
#include <vulkan_headers.hpp>

namespace Engine
{
	extern const Array<vk::IndexType, 3> m_index_types;
	extern const Array<vk::PrimitiveTopology, 5> m_primitive_topologies;
	extern const Array<vk::Format, 19> m_shader_data_types;
	extern const Array<vk::ComponentSwizzle, 7> m_swizzle_components;
	extern const Array<vk::SamplerAddressMode, 5> m_address_modes;
	extern const Array<vk::CompareOp, 8> m_compare_funcs;
	extern const Array<vk::StencilOp, 8> m_stencil_ops;
	extern const Array<vk::BlendFactor, 14> m_blend_factors;
	extern const Array<vk::BlendOp, 5> m_blend_ops;
	extern const Array<vk::PolygonMode, 3> m_poligon_modes;
	extern const Array<vk::CullModeFlagBits, 3> m_cull_modes;
	extern const Array<vk::FrontFace, 2> m_front_faces;
	extern const Array<vk::ImageAspectFlags, 5> m_image_aspects;


	template<typename ResultType, typename ElementType, std::size_t size>
	ResultType vulkan_type_to_engine_type(const Array<ElementType, size>& containter, const ElementType& elem)
	{
		for (std::size_t i = 0; i < size; i++)
		{
			if (elem == containter[i])
				return static_cast<ResultType>(i);
		}

		throw std::runtime_error("VulkanAPI: Failed to convert Vulkan type to Engine type");
	}


#define DECLARE_GETTER(a, b, array)                                                                                              \
	inline a get_type(const b& in_type)                                                                                          \
	{                                                                                                                            \
		return array[static_cast<EnumerateType>(in_type)];                                                                       \
	}

	DECLARE_GETTER(vk::FrontFace, FrontFace, m_front_faces);
	DECLARE_GETTER(vk::ComponentSwizzle, Swizzle, m_swizzle_components);
	DECLARE_GETTER(vk::SamplerAddressMode, SamplerAddressMode, m_address_modes);
	DECLARE_GETTER(vk::CompareOp, CompareFunc, m_compare_funcs);
	DECLARE_GETTER(vk::StencilOp, StencilOp, m_stencil_ops);
	DECLARE_GETTER(vk::BlendOp, BlendOp, m_blend_ops);
	DECLARE_GETTER(vk::PrimitiveTopology, PrimitiveTopology, m_primitive_topologies);
	DECLARE_GETTER(vk::PolygonMode, PolygonMode, m_poligon_modes);
	DECLARE_GETTER(vk::CullModeFlagBits, CullMode, m_cull_modes);

	inline vk::BlendFactor get_type(BlendFunc func, bool is_for_alpha)
	{
		switch (func)
		{
			case BlendFunc::Zero:
				return vk::BlendFactor::eZero;

			case BlendFunc::One:
				return vk::BlendFactor::eOne;

			case BlendFunc::SrcColor:
				return vk::BlendFactor::eSrcColor;

			case BlendFunc::OneMinusSrcColor:
				return vk::BlendFactor::eOneMinusSrcColor;

			case BlendFunc::DstColor:
				return vk::BlendFactor::eDstColor;

			case BlendFunc::OneMinusDstColor:
				return vk::BlendFactor::eOneMinusDstColor;

			case BlendFunc::SrcAlpha:
				return vk::BlendFactor::eSrcAlpha;

			case BlendFunc::OneMinusSrcAlpha:
				return vk::BlendFactor::eOneMinusSrcAlpha;

			case BlendFunc::DstAlpha:
				return vk::BlendFactor::eDstAlpha;

			case BlendFunc::OneMinusDstAlpha:
				return vk::BlendFactor::eOneMinusDstAlpha;

			case BlendFunc::BlendFactor:
				return is_for_alpha ? vk::BlendFactor::eConstantAlpha : vk::BlendFactor::eConstantColor;
			case BlendFunc::OneMinusBlendFactor:
				return is_for_alpha ? vk::BlendFactor::eOneMinusConstantAlpha : vk::BlendFactor::eOneMinusConstantColor;

			default:
				return vk::BlendFactor::eZero;
		}
	}

#undef DECLARE_GETTER
}// namespace Engine
