#pragma once
#include <Core/color_format.hpp>
#include <Core/rhi_initializers.hpp>
#include <stdexcept>
#include <vulkan_headers.hpp>

namespace Engine
{
    extern const Array<vk::IndexType, 3> _M_index_types;
    extern const Array<vk::PrimitiveTopology, 11> _M_primitive_topologies;
    extern const Array<vk::Format, 19> _M_shader_data_types;
    extern const Array<vk::ComponentSwizzle, 7> _M_swizzle_components;
    extern const Array<vk::SamplerAddressMode, 5> _M_wrap_values;
    extern const Array<vk::CompareOp, 8> _M_compare_funcs;
    extern const Array<vk::StencilOp, 8> _M_stencil_ops;
    extern const Array<vk::BlendFactor, 14> _M_blend_factors;
    extern const Array<vk::BlendOp, 5> _M_blend_ops;
    extern const Array<vk::PrimitiveTopology, 11> _M_primitive_topologies;
    extern const Array<vk::PolygonMode, 3> _M_poligon_modes;
    extern const Array<vk::CullModeFlagBits, 4> _M_cull_modes;
    extern const Array<vk::FrontFace, 2> _M_front_faces;
    extern const Array<vk::LogicOp, 17> _M_logic_ops;
    extern const Array<vk::ImageAspectFlags, 5> _M_image_aspects;


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


#define DECLARE_GETTER(a, b, array)                                                                                    \
    inline a get_type(const b& in_type)                                                                                \
    {                                                                                                                  \
        return array[static_cast<EnumerateType>(in_type)];                                                             \
    }

    DECLARE_GETTER(vk::IndexType, IndexBufferComponent, _M_index_types);
    DECLARE_GETTER(vk::LogicOp, LogicOp, _M_logic_ops);
    DECLARE_GETTER(vk::FrontFace, FrontFace, _M_front_faces);
    DECLARE_GETTER(vk::ComponentSwizzle, SwizzleValue, _M_swizzle_components);
    DECLARE_GETTER(vk::SamplerAddressMode, WrapValue, _M_wrap_values);
    DECLARE_GETTER(vk::CompareOp, CompareFunc, _M_compare_funcs);
    DECLARE_GETTER(vk::StencilOp, StencilOp, _M_stencil_ops);
    DECLARE_GETTER(vk::BlendFactor, BlendFunc, _M_blend_factors);
    DECLARE_GETTER(vk::BlendOp, BlendOp, _M_blend_ops);
    DECLARE_GETTER(vk::PrimitiveTopology, PrimitiveTopology, _M_primitive_topologies);
    DECLARE_GETTER(vk::PolygonMode, PolygonMode, _M_poligon_modes);
    DECLARE_GETTER(vk::CullModeFlagBits, CullMode, _M_cull_modes);
    DECLARE_GETTER(vk::ImageAspectFlags, ColorFormatAspect, _M_image_aspects);

#undef DECLARE_GETTER
}// namespace Engine
