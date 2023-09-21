#include <opengl_types.hpp>


namespace Engine
{

    template<size_t size, typename InputType, typename ResultType = GLint>
    const Array<ResultType, size>
    generate_array(const std::initializer_list<const std::pair<InputType, ResultType>>& list)
    {
        Array<ResultType, size> out_array;

        for (auto& pair : list)
        {
            if (static_cast<EnumerateType>(pair.first) < size)
                out_array[static_cast<EnumerateType>(pair.first)] = pair.second;
        }
        return out_array;
    }

#define DECLARE_TYPE(type, name, size, ...)                                                                            \
    const Array<GLint, size> _M_##name = generate_array<size, type>({__VA_ARGS__})


    DECLARE_TYPE(TextureType, texture_types, 2, {TextureType::Texture2D, GL_TEXTURE_2D},
                 {TextureType::TextureCubeMap, GL_TEXTURE_CUBE_MAP});


    DECLARE_TYPE(CompareFunc, compare_funcs, 8, {CompareFunc::Always, GL_ALWAYS}, {CompareFunc::Lequal, GL_LEQUAL},
                 {CompareFunc::Gequal, GL_GEQUAL}, {CompareFunc::Less, GL_LEQUAL}, {CompareFunc::Greater, GL_GREATER},
                 {CompareFunc::Equal, GL_EQUAL}, {CompareFunc::NotEqual, GL_NOTEQUAL}, {CompareFunc::Never, GL_NEVER});


    DECLARE_TYPE(CompareMode, compare_modes, 2, {CompareMode::None, GL_NONE},
                 {CompareMode::RefToTexture, GL_COMPARE_REF_TO_TEXTURE});

    DECLARE_TYPE(TextureFilter, texture_filters, 2, {TextureFilter::Nearest, GL_NEAREST},
                 {TextureFilter::Linear, GL_LINEAR});

    DECLARE_TYPE(SamplerMipmapMode, sampler_modes, 2, {SamplerMipmapMode::Nearest, GL_NEAREST},
                 {SamplerMipmapMode::Linear, GL_LINEAR});

    DECLARE_TYPE(SwizzleValue, swizzle_values, 7, {SwizzleValue::One, GL_ONE}, {SwizzleValue::Zero, GL_ZERO},
                 {SwizzleValue::Identity, 0}, {SwizzleValue::R, GL_RED}, {SwizzleValue::G, GL_GREEN},
                 {SwizzleValue::B, GL_BLUE}, {SwizzleValue::A, GL_ALPHA});

    DECLARE_TYPE(WrapValue, wrap_values, 5, {WrapValue::Repeat, GL_REPEAT},
                 {WrapValue::MirroredRepeat, GL_MIRRORED_REPEAT}, {WrapValue::ClampToBorder, GL_CLAMP_TO_BORDER},
                 {WrapValue::ClampToEdge, GL_CLAMP_TO_EDGE},
                 {WrapValue::MirrorClampToEdge, GL_MIRROR_CLAMP_TO_EDGE_EXT});

    DECLARE_TYPE(TextureCubeMapFace, cube_faces, 6, {TextureCubeMapFace::Back, GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
                 {TextureCubeMapFace::Front, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
                 {TextureCubeMapFace::Up, GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
                 {TextureCubeMapFace::Down, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
                 {TextureCubeMapFace::Left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
                 {TextureCubeMapFace::Right, GL_TEXTURE_CUBE_MAP_POSITIVE_X});

    DECLARE_TYPE(IndexBufferComponent, index_components, 3, {IndexBufferComponent::UnsignedByte, GL_UNSIGNED_BYTE},
                 {IndexBufferComponent::UnsignedInt, GL_UNSIGNED_INT},
                 {IndexBufferComponent::UnsignedShort, GL_UNSIGNED_SHORT});


    const Array<ShaderType, 19> _M_shader_types = generate_array<19, typeof(ShaderDataType::Bool), ShaderType>({
            {ShaderDataType::Bool, ShaderType{1, GL_BOOL}},
            {ShaderDataType::Int, ShaderType{1, GL_INT}},
            {ShaderDataType::UInt, ShaderType{1, GL_UNSIGNED_INT}},
            {ShaderDataType::Float, ShaderType{1, GL_FLOAT}},
            {ShaderDataType::Vec2, ShaderType{2, GL_FLOAT}},
            {ShaderDataType::Vec3, ShaderType{3, GL_FLOAT}},
            {ShaderDataType::Vec4, ShaderType{4, GL_FLOAT}},
            {ShaderDataType::IVec2, ShaderType{2, GL_INT}},
            {ShaderDataType::IVec3, ShaderType{3, GL_INT}},
            {ShaderDataType::IVec4, ShaderType{4, GL_INT}},
            {ShaderDataType::UVec2, ShaderType{2, GL_UNSIGNED_INT}},
            {ShaderDataType::UVec3, ShaderType{3, GL_UNSIGNED_INT}},
            {ShaderDataType::UVec4, ShaderType{4, GL_UNSIGNED_INT}},
            {ShaderDataType::BVec2, ShaderType{2, GL_BOOL}},
            {ShaderDataType::BVec3, ShaderType{3, GL_BOOL}},
            {ShaderDataType::BVec4, ShaderType{4, GL_BOOL}},
            {ShaderDataType::Mat2, ShaderType{4, GL_FLOAT}},
            {ShaderDataType::Mat3, ShaderType{9, GL_FLOAT}},
            {ShaderDataType::Mat4, ShaderType{16, GL_FLOAT}},
    });

    DECLARE_TYPE(StencilOp, stencil_options, 8, {StencilOp::Decr, GL_DECR}, {StencilOp::DecrWrap, GL_DECR_WRAP},
                 {StencilOp::Incr, GL_INCR}, {StencilOp::IncrWrap, GL_INCR_WRAP}, {StencilOp::Invert, GL_INVERT},
                 {StencilOp::Keep, GL_KEEP}, {StencilOp::Replace, GL_REPLACE}, {StencilOp::Zero, GL_ZERO});


    DECLARE_TYPE(PrimitiveTopology, primitive_topologies, 11, {PrimitiveTopology::PointList, GL_POINTS},
                 {PrimitiveTopology::LineList, GL_LINES},
                 {PrimitiveTopology::LineListWithAdjacency, GL_LINES_ADJACENCY},
                 {PrimitiveTopology::LineStrip, GL_LINE_STRIP},
                 {PrimitiveTopology::LineStripWithAdjacency, GL_LINE_STRIP_ADJACENCY},
                 {PrimitiveTopology::TriangleList, GL_TRIANGLES}, {PrimitiveTopology::TriangleFan, GL_TRIANGLE_FAN},
                 {PrimitiveTopology::TriangleStrip, GL_TRIANGLE_STRIP},
                 {PrimitiveTopology::TriangleListWithAdjacency, GL_TRIANGLES_ADJACENCY},
                 {PrimitiveTopology::TriangleStripWithAdjacency, GL_TRIANGLE_STRIP_ADJACENCY},
                 {PrimitiveTopology::PatchList, GL_PATCHES});

    DECLARE_TYPE(BlendFunc, blend_funcs, 14, {BlendFunc::Zero, GL_ZERO}, {BlendFunc::One, GL_ONE},
                 {BlendFunc::SrcColor, GL_SRC_COLOR}, {BlendFunc::OneMinusSrcColor, GL_ONE_MINUS_SRC_COLOR},
                 {BlendFunc::DstColor, GL_DST_COLOR}, {BlendFunc::OneMinusDstColor, GL_ONE_MINUS_DST_COLOR},
                 {BlendFunc::SrcAlpha, GL_SRC_ALPHA}, {BlendFunc::OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA},
                 {BlendFunc::DstAlpha, GL_DST_ALPHA}, {BlendFunc::OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA},
                 {BlendFunc::ConstantColor, GL_CONSTANT_COLOR},
                 {BlendFunc::OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR},
                 {BlendFunc::ConstantAlpha, GL_CONSTANT_ALPHA},
                 {BlendFunc::OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA});

    DECLARE_TYPE(BlendOp, blend_ops, 5, {BlendOp::Add, GL_FUNC_ADD}, {BlendOp::Subtract, GL_FUNC_SUBTRACT},
                 {BlendOp::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT}, {BlendOp::Min, GL_MIN}, {BlendOp::Max, GL_MAX});

    DECLARE_TYPE(CullMode, cull_modes, 4, {CullMode::None, GL_NONE}, {CullMode::Back, GL_BACK},
                 {CullMode::Front, GL_FRONT}, {CullMode::FrontAndBack, GL_FRONT_AND_BACK});


}// namespace Engine
