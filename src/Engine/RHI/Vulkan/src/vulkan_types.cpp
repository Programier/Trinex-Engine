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

    const Array<vk::IndexType, 3> _M_index_types = generate_array<vk::IndexType, 3, IndexBufferComponent>({
            {IndexBufferComponent::UnsignedInt, vk::IndexType::eUint32},
            {IndexBufferComponent::UnsignedShort, vk::IndexType::eUint16},
            {IndexBufferComponent::UnsignedByte, vk::IndexType::eUint8EXT},
    });

    const vk::Format _M_shader_data_types[19] = {
            vk::Format::eUndefined,         //  BOOL
            vk::Format::eR32Sint,           //  INT
            vk::Format::eR32Uint,           //  UINT
            vk::Format::eR32Sfloat,         //  FLOAT
            vk::Format::eR32G32Sfloat,      //  VEC2
            vk::Format::eR32G32B32Sfloat,   //  VEC3
            vk::Format::eR32G32B32A32Sfloat,//  VEC4
            vk::Format::eR32G32Sint,        //  IVEC2
            vk::Format::eR32G32B32Sint,     //  IVEC3
            vk::Format::eR32G32B32A32Sint,  //  IVEC4
            vk::Format::eR32G32Uint,        //  UVEC2
            vk::Format::eR32G32B32Uint,     //  UVEC3
            vk::Format::eR32G32B32A32Uint,  //  UVEC4
            vk::Format::eUndefined,         //  BVEC2
            vk::Format::eUndefined,         //  BVEC3
            vk::Format::eUndefined,         //  BVEC4
            vk::Format::eUndefined,         //  MAT2
            vk::Format::eUndefined,         //  MAT3
            vk::Format::eUndefined,         //  MAT4
    };

    const Array<vk::ComponentSwizzle, 7> _M_swizzle_components = {
            vk::ComponentSwizzle::eIdentity,// Identity = 0
            vk::ComponentSwizzle::eZero,    // Zero = 1
            vk::ComponentSwizzle::eOne,     // One = 2
            vk::ComponentSwizzle::eR,       // R = 3
            vk::ComponentSwizzle::eG,       // G = 4
            vk::ComponentSwizzle::eB,       // B = 5
            vk::ComponentSwizzle::eA,       // A = 6
    };

    const Array<vk::SamplerAddressMode, 5> _M_wrap_values = {
            vk::SamplerAddressMode::eRepeat,           // Repeat = 0
            vk::SamplerAddressMode::eClampToEdge,      // ClampToEdge = 1
            vk::SamplerAddressMode::eClampToBorder,    // ClampToBorder = 2
            vk::SamplerAddressMode::eMirroredRepeat,   // MirroredRepeat = 3
            vk::SamplerAddressMode::eMirrorClampToEdge,// MirrorClampToEdge = 4
    };

    const Array<vk::Filter, 2> _M_texture_filters = generate_array<vk::Filter, 2, TextureFilter>({
            {TextureFilter::Nearest, vk::Filter::eNearest},
            {TextureFilter::Linear, vk::Filter::eLinear},
    });

    const Array<vk::SamplerMipmapMode, 2> _M_sampler_mipmap_modes =
            generate_array<vk::SamplerMipmapMode, 2, SamplerMipmapMode>({
                    {SamplerMipmapMode::Nearest, vk::SamplerMipmapMode::eNearest},
                    {SamplerMipmapMode::Linear, vk::SamplerMipmapMode::eLinear},
            });


    const Array<vk::CompareOp, 8> _M_compare_funcs = generate_array<vk::CompareOp, 8, CompareFunc>({
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


    const Array<vk::StencilOp, 8> _M_stencil_ops = generate_array<vk::StencilOp, 8, StencilOp>({
            {StencilOp::Keep, vk::StencilOp::eKeep},
            {StencilOp::Zero, vk::StencilOp::eZero},
            {StencilOp::Replace, vk::StencilOp::eReplace},
            {StencilOp::Incr, vk::StencilOp::eIncrementAndClamp},
            {StencilOp::IncrWrap, vk::StencilOp::eIncrementAndWrap},
            {StencilOp::Decr, vk::StencilOp::eDecrementAndClamp},
            {StencilOp::DecrWrap, vk::StencilOp::eDecrementAndWrap},
            {StencilOp::Invert, vk::StencilOp::eInvert},
    });

    const Array<vk::BlendFactor, 14> _M_blend_factors = generate_array<vk::BlendFactor, 14, BlendFunc>({
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

    const Array<vk::BlendOp, 5> _M_blend_ops = generate_array<vk::BlendOp, 5, BlendOp>({
            {BlendOp::Add, vk::BlendOp::eAdd},
            {BlendOp::Subtract, vk::BlendOp::eSubtract},
            {BlendOp::ReverseSubtract, vk::BlendOp::eReverseSubtract},
            {BlendOp::Min, vk::BlendOp::eMin},
            {BlendOp::Max, vk::BlendOp::eMax},
    });


    const Array<vk::PrimitiveTopology, 11> _M_primitive_topologies =
            generate_array<vk::PrimitiveTopology, 11, PrimitiveTopology>({
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

    const Array<vk::PolygonMode, 3> _M_poligon_modes = generate_array<vk::PolygonMode, 3, PolygonMode>({
            {PolygonMode::Fill, vk::PolygonMode::eFill},
            {PolygonMode::Line, vk::PolygonMode::eLine},
            {PolygonMode::Point, vk::PolygonMode::ePoint},
    });

    const Array<vk::CullModeFlagBits, 4> _M_cull_modes = generate_array<vk::CullModeFlagBits, 4, CullMode>({
            {CullMode::None, vk::CullModeFlagBits::eNone},
            {CullMode::Back, vk::CullModeFlagBits::eBack},
            {CullMode::Front, vk::CullModeFlagBits::eFront},
            {CullMode::FrontAndBack, vk::CullModeFlagBits::eFrontAndBack},
    });

    const Array<vk::FrontFace, 2> _M_front_faces = generate_array<vk::FrontFace, 2, FrontFace>({
            {FrontFace::ClockWise, vk::FrontFace::eClockwise},
            {FrontFace::CounterClockWise, vk::FrontFace::eCounterClockwise},
    });

    const Array<vk::LogicOp, 17> _M_logic_ops = generate_array<vk::LogicOp, 17, LogicOp>({
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

    const Array<vk::ImageAspectFlags, 5> _M_image_aspects = generate_array<vk::ImageAspectFlags, 5, ColorFormatAspect>({
            {ColorFormatAspect::Color, vk::ImageAspectFlagBits::eColor},
            {ColorFormatAspect::Depth, vk::ImageAspectFlagBits::eDepth},
            {ColorFormatAspect::Stencil, vk::ImageAspectFlagBits::eStencil},
            {ColorFormatAspect::DepthStencil, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil},
    });
}// namespace Engine
