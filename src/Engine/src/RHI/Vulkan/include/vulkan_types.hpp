#pragma once
#include <Core/enums.hpp>
#include <stdexcept>
#include <vulkan_headers.hpp>

namespace Engine
{
    extern const Array<vk::IndexType, 3> m_index_types;
    extern const Array<vk::PrimitiveTopology, 11> m_primitive_topologies;
    extern const Array<vk::Format, 19> m_shader_data_types;
    extern const Array<vk::ComponentSwizzle, 7> m_swizzle_components;
    extern const Array<vk::SamplerAddressMode, 5> m_wrap_values;
    extern const Array<vk::CompareOp, 8> m_compare_funcs;
    extern const Array<vk::StencilOp, 8> m_stencil_ops;
    extern const Array<vk::BlendFactor, 14> m_blend_factors;
    extern const Array<vk::BlendOp, 5> m_blend_ops;
    extern const Array<vk::PrimitiveTopology, 11> m_primitive_topologies;
    extern const Array<vk::PolygonMode, 3> m_poligon_modes;
    extern const Array<vk::CullModeFlagBits, 4> m_cull_modes;
    extern const Array<vk::FrontFace, 2> m_front_faces;
    extern const Array<vk::LogicOp, 17> m_logic_ops;
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

    DECLARE_GETTER(vk::LogicOp, LogicOp, m_logic_ops);
    DECLARE_GETTER(vk::FrontFace, FrontFace, m_front_faces);
    DECLARE_GETTER(vk::ComponentSwizzle, Swizzle, m_swizzle_components);
    DECLARE_GETTER(vk::SamplerAddressMode, WrapValue, m_wrap_values);
    DECLARE_GETTER(vk::CompareOp, CompareFunc, m_compare_funcs);
    DECLARE_GETTER(vk::StencilOp, StencilOp, m_stencil_ops);
    DECLARE_GETTER(vk::BlendFactor, BlendFunc, m_blend_factors);
    DECLARE_GETTER(vk::BlendOp, BlendOp, m_blend_ops);
    DECLARE_GETTER(vk::PrimitiveTopology, PrimitiveTopology, m_primitive_topologies);
    DECLARE_GETTER(vk::PolygonMode, PolygonMode, m_poligon_modes);
    DECLARE_GETTER(vk::CullModeFlagBits, CullMode, m_cull_modes);

#undef DECLARE_GETTER
}// namespace Engine
