#include <vulkan_types.hpp>

namespace Engine
{

    template<typename OutType, size_t size, typename InputType>
    const Array<OutType, size> generate_array(const std::initializer_list<const std::pair<InputType, OutType>>& list)
    {
        Array<OutType, size> out_array;

        for (auto& pair : list)
        {
            if (static_cast<EnumerateType>(pair.first) < size)
                out_array[static_cast<EnumerateType>(pair.first)] = pair.second;
        }
        return out_array;
    }

    const Array<vk::ComponentSwizzle, 7> m_swizzle_components = {
            vk::ComponentSwizzle::eIdentity,// Identity = 0
            vk::ComponentSwizzle::eZero,    // Zero = 1
            vk::ComponentSwizzle::eOne,     // One = 2
            vk::ComponentSwizzle::eR,       // R = 3
            vk::ComponentSwizzle::eG,       // G = 4
            vk::ComponentSwizzle::eB,       // B = 5
            vk::ComponentSwizzle::eA,       // A = 6
    };

    const Array<vk::SamplerAddressMode, 5> m_wrap_values = {
            vk::SamplerAddressMode::eRepeat,           // Repeat = 0
            vk::SamplerAddressMode::eClampToEdge,      // ClampToEdge = 1
            vk::SamplerAddressMode::eClampToBorder,    // ClampToBorder = 2
            vk::SamplerAddressMode::eMirroredRepeat,   // MirroredRepeat = 3
            vk::SamplerAddressMode::eMirrorClampToEdge,// MirrorClampToEdge = 4
    };


    const Array<vk::CompareOp, 8> m_compare_funcs = generate_array<vk::CompareOp, 8, CompareFunc>({
            {CompareFunc::Always, vk::CompareOp::eAlways},
            {CompareFunc::Lequal, vk::CompareOp::eLessOrEqual},
            {CompareFunc::Gequal, vk::CompareOp::eGreaterOrEqual},
            {CompareFunc::Lequal, vk::CompareOp::eLessOrEqual},
            {CompareFunc::Less, vk::CompareOp::eLess},
            {CompareFunc::Greater, vk::CompareOp::eGreater},
            {CompareFunc::Equal, vk::CompareOp::eEqual},
            {CompareFunc::NotEqual, vk::CompareOp::eNotEqual},
            {CompareFunc::Never, vk::CompareOp::eNever},
    });


    const Array<vk::StencilOp, 8> m_stencil_ops = generate_array<vk::StencilOp, 8, StencilOp>({
            {StencilOp::Keep, vk::StencilOp::eKeep},
            {StencilOp::Zero, vk::StencilOp::eZero},
            {StencilOp::Replace, vk::StencilOp::eReplace},
            {StencilOp::Incr, vk::StencilOp::eIncrementAndClamp},
            {StencilOp::IncrWrap, vk::StencilOp::eIncrementAndWrap},
            {StencilOp::Decr, vk::StencilOp::eDecrementAndClamp},
            {StencilOp::DecrWrap, vk::StencilOp::eDecrementAndWrap},
            {StencilOp::Invert, vk::StencilOp::eInvert},
    });

    const Array<vk::BlendFactor, 14> m_blend_factors = generate_array<vk::BlendFactor, 14, BlendFunc>({
            {BlendFunc::Zero, vk::BlendFactor::eZero},
            {BlendFunc::One, vk::BlendFactor::eOne},
            {BlendFunc::SrcColor, vk::BlendFactor::eSrcColor},
            {BlendFunc::OneMinusSrcColor, vk::BlendFactor::eOneMinusSrcColor},
            {BlendFunc::DstColor, vk::BlendFactor::eDstColor},
            {BlendFunc::OneMinusDstColor, vk::BlendFactor::eOneMinusDstColor},
            {BlendFunc::SrcAlpha, vk::BlendFactor::eSrcAlpha},
            {BlendFunc::OneMinusSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha},
            {BlendFunc::DstAlpha, vk::BlendFactor::eDstAlpha},
            {BlendFunc::OneMinusDstAlpha, vk::BlendFactor::eOneMinusDstAlpha},
            {BlendFunc::ConstantColor, vk::BlendFactor::eConstantColor},
            {BlendFunc::OneMinusConstantColor, vk::BlendFactor::eOneMinusConstantColor},
            {BlendFunc::ConstantAlpha, vk::BlendFactor::eConstantAlpha},
            {BlendFunc::OneMinusConstantAlpha, vk::BlendFactor::eOneMinusConstantAlpha},
    });

    const Array<vk::BlendOp, 5> m_blend_ops = generate_array<vk::BlendOp, 5, BlendOp>({
            {BlendOp::Add, vk::BlendOp::eAdd},
            {BlendOp::Subtract, vk::BlendOp::eSubtract},
            {BlendOp::ReverseSubtract, vk::BlendOp::eReverseSubtract},
            {BlendOp::Min, vk::BlendOp::eMin},
            {BlendOp::Max, vk::BlendOp::eMax},
    });


    const Array<vk::PrimitiveTopology, 11> m_primitive_topologies = generate_array<vk::PrimitiveTopology, 11, PrimitiveTopology>({
            {PrimitiveTopology::TriangleList, vk::PrimitiveTopology::eTriangleList},
            {PrimitiveTopology::PointList, vk::PrimitiveTopology::ePointList},
            {PrimitiveTopology::LineList, vk::PrimitiveTopology::eLineList},
            {PrimitiveTopology::LineStrip, vk::PrimitiveTopology::eLineStrip},
            {PrimitiveTopology::TriangleStrip, vk::PrimitiveTopology::eTriangleStrip},
            {PrimitiveTopology::TriangleFan, vk::PrimitiveTopology::eTriangleFan},
            {PrimitiveTopology::LineListWithAdjacency, vk::PrimitiveTopology::eLineListWithAdjacency},
            {PrimitiveTopology::LineStripWithAdjacency, vk::PrimitiveTopology::eLineStripWithAdjacency},
            {PrimitiveTopology::TriangleListWithAdjacency, vk::PrimitiveTopology::eTriangleListWithAdjacency},
            {PrimitiveTopology::TriangleStripWithAdjacency, vk::PrimitiveTopology::eTriangleStripWithAdjacency},
            {PrimitiveTopology::PatchList, vk::PrimitiveTopology::ePatchList},
    });

    const Array<vk::PolygonMode, 3> m_poligon_modes = generate_array<vk::PolygonMode, 3, PolygonMode>({
            {PolygonMode::Fill, vk::PolygonMode::eFill},
            {PolygonMode::Line, vk::PolygonMode::eLine},
            {PolygonMode::Point, vk::PolygonMode::ePoint},
    });

    const Array<vk::CullModeFlagBits, 4> m_cull_modes = generate_array<vk::CullModeFlagBits, 4, CullMode>({
            {CullMode::None, vk::CullModeFlagBits::eNone},
            {CullMode::Back, vk::CullModeFlagBits::eBack},
            {CullMode::Front, vk::CullModeFlagBits::eFront},
            {CullMode::FrontAndBack, vk::CullModeFlagBits::eFrontAndBack},
    });

    const Array<vk::FrontFace, 2> m_front_faces = generate_array<vk::FrontFace, 2, FrontFace>({
            // So as viewport is flipped, inverse FrontFace values
            {FrontFace::ClockWise, vk::FrontFace::eCounterClockwise},
            {FrontFace::CounterClockWise, vk::FrontFace::eClockwise},
    });

    const Array<vk::LogicOp, 17> m_logic_ops = generate_array<vk::LogicOp, 17, LogicOp>({
            {LogicOp::Clear, vk::LogicOp::eClear},
            {LogicOp::And, vk::LogicOp::eAnd},
            {LogicOp::AndReverse, vk::LogicOp::eAndReverse},
            {LogicOp::Copy, vk::LogicOp::eCopy},
            {LogicOp::AndInverted, vk::LogicOp::eAndInverted},
            {LogicOp::NoOp, vk::LogicOp::eNoOp},
            {LogicOp::Xor, vk::LogicOp::eXor},
            {LogicOp::Or, vk::LogicOp::eOr},
            {LogicOp::Nor, vk::LogicOp::eNor},
            {LogicOp::Equivalent, vk::LogicOp::eEquivalent},
            {LogicOp::Invert, vk::LogicOp::eInvert},
            {LogicOp::OrReverse, vk::LogicOp::eOrReverse},
            {LogicOp::CopyInverted, vk::LogicOp::eCopyInverted},
            {LogicOp::OrInverted, vk::LogicOp::eOrInverted},
            {LogicOp::Nand, vk::LogicOp::eNand},
            {LogicOp::Set, vk::LogicOp::eSet},
    });
}// namespace Engine