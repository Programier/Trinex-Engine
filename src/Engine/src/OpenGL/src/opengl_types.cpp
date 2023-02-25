#include <opengl_types.hpp>


const std::unordered_map<TextureType, GLuint> _M_types = {
        {TextureType::Texture2D, GL_TEXTURE_2D},
        {TextureType::Texture3D, GL_TEXTURE_3D},
        {TextureType::TextureCubeMap, GL_TEXTURE_CUBE_MAP},
};

const std::unordered_map<PixelType, GLuint> _M_pixel_types = {
        {PixelType::RGB, GL_RGB},
        {PixelType::RGBA, GL_RGBA},
        {PixelType::Depth, GL_DEPTH_COMPONENT},
        {PixelType::Red, GL_RED},
        {PixelType::DepthComponent16, GL_DEPTH_COMPONENT16},
        {PixelType::DepthComponent24, GL_DEPTH_COMPONENT24},
        {PixelType::DepthComponent32f, GL_DEPTH_COMPONENT32F},
        {PixelType::Depth24Stencil8, GL_DEPTH24_STENCIL8},
        {PixelType::Depth32fStencil8, GL_DEPTH32F_STENCIL8},
};


const std::unordered_map<BufferValueType, GLuint> _M_buffer_value_types = {
        {BufferValueType::Float, GL_FLOAT},
        {BufferValueType::UnsignedByte, GL_UNSIGNED_BYTE},
        {BufferValueType::UnsignedShort, GL_UNSIGNED_SHORT},
        {BufferValueType::UnsignedInt, GL_UNSIGNED_INT},
        {BufferValueType::Short, GL_SHORT},
        {BufferValueType::Int, GL_INT},
        {BufferValueType::Byte, GL_BYTE},
        {BufferValueType::HalfFloat, GL_HALF_FLOAT},
        {BufferValueType::UnsignedShort565, GL_UNSIGNED_SHORT_5_6_5},
        {BufferValueType::UnsignedShort4444, GL_UNSIGNED_SHORT_4_4_4_4},
        {BufferValueType::UnsignedShort5551, GL_UNSIGNED_SHORT_5_5_5_1},
        {BufferValueType::UnsignedInt10f11f11fRev, GL_UNSIGNED_INT_10F_11F_11F_REV},
        {BufferValueType::UnsignedInt2101010Rev, GL_UNSIGNED_INT_2_10_10_10_REV},
        {BufferValueType::UnsignedInt248, GL_UNSIGNED_INT_24_8},
        {BufferValueType::UnsignedInt5999Rev, GL_UNSIGNED_INT_5_9_9_9_REV},
        {BufferValueType::Float32UnsignedInt248Rev, GL_FLOAT_32_UNSIGNED_INT_24_8_REV},
};

const std::unordered_map<typeof(ShaderDataType::Int), std::pair<byte, GLuint>> _M_shader_types{
        {ShaderDataType::Bool, {1, GL_BOOL}},          {ShaderDataType::Int, {1, GL_INT}},
        {ShaderDataType::UInt, {1, GL_UNSIGNED_INT}},  {ShaderDataType::Float, {1, GL_FLOAT}},
        {ShaderDataType::Vec2, {2, GL_FLOAT}},         {ShaderDataType::Vec3, {3, GL_FLOAT}},
        {ShaderDataType::Vec4, {4, GL_FLOAT}},         {ShaderDataType::IVec2, {2, GL_INT}},
        {ShaderDataType::IVec3, {3, GL_INT}},          {ShaderDataType::IVec4, {4, GL_INT}},
        {ShaderDataType::UVec2, {2, GL_UNSIGNED_INT}}, {ShaderDataType::UVec3, {3, GL_UNSIGNED_INT}},
        {ShaderDataType::UVec4, {4, GL_UNSIGNED_INT}}, {ShaderDataType::BVec2, {2, GL_BOOL}},
        {ShaderDataType::BVec3, {3, GL_BOOL}},         {ShaderDataType::BVec4, {4, GL_BOOL}},
        {ShaderDataType::Mat2, {4, GL_FLOAT}},         {ShaderDataType::Mat3, {9, GL_FLOAT}},
        {ShaderDataType::Mat4, {16, GL_FLOAT}},
};

const std::unordered_map<IndexBufferComponent, GLuint> _M_index_buffer_components = {
        {IndexBufferComponent::UnsignedByte, GL_UNSIGNED_BYTE},
        {IndexBufferComponent::UnsignedInt, GL_UNSIGNED_INT},
        {IndexBufferComponent::UnsignedShort, GL_UNSIGNED_SHORT},
};

const std::unordered_map<CompareFunc, GLuint> _M_compare_funcs = {
        {CompareFunc::Lequal, GL_LEQUAL},   {CompareFunc::Gequal, GL_GEQUAL}, {CompareFunc::Less, GL_LESS},
        {CompareFunc::Greater, GL_GREATER}, {CompareFunc::Equal, GL_EQUAL},   {CompareFunc::NotEqual, GL_NOTEQUAL},
        {CompareFunc::Always, GL_ALWAYS},   {CompareFunc::Never, GL_NEVER},
};

const std::unordered_map<GLint, CompareFunc> _M_revert_compare_funcs = {
        {GL_LEQUAL, CompareFunc::Lequal},   {GL_GEQUAL, CompareFunc::Gequal}, {GL_LESS, CompareFunc::Less},
        {GL_GREATER, CompareFunc::Greater}, {GL_EQUAL, CompareFunc::Equal},   {GL_NOTEQUAL, CompareFunc::NotEqual},
        {GL_ALWAYS, CompareFunc::Always},   {GL_NEVER, CompareFunc::Never},
};

const std::unordered_map<CompareMode, GLuint> _M_compare_modes = {
        {CompareMode::None, GL_NONE},
        {CompareMode::RefToTexture, GL_COMPARE_REF_TO_TEXTURE},
};

const std::unordered_map<TextureFilter, GLuint> _M_texture_filters = {
        {TextureFilter::Nearest, GL_NEAREST},
        {TextureFilter::Linear, GL_LINEAR},
        {TextureFilter::NearestMipmapNearest, GL_NEAREST_MIPMAP_NEAREST},
        {TextureFilter::NearestMipmapLinear, GL_NEAREST_MIPMAP_LINEAR},
        {TextureFilter::LinearMipmapNearest, GL_LINEAR_MIPMAP_NEAREST},
        {TextureFilter::LinearMipmapLinear, GL_LINEAR_MIPMAP_LINEAR},
};


const std::unordered_map<GLint, TextureFilter> _M_reverse_texture_filters = {
        {GL_NEAREST, TextureFilter::Nearest},
        {GL_LINEAR, TextureFilter::Linear},
        {GL_NEAREST_MIPMAP_NEAREST, TextureFilter::NearestMipmapNearest},
        {GL_NEAREST_MIPMAP_LINEAR, TextureFilter::NearestMipmapLinear},
        {GL_LINEAR_MIPMAP_NEAREST, TextureFilter::LinearMipmapNearest},
        {GL_LINEAR_MIPMAP_LINEAR, TextureFilter::LinearMipmapLinear},
};

const std::unordered_map<SwizzleRGBA::SwizzleValue, GLint> _M_swizzle_values = {
        {SwizzleRGBA::SwizzleValue::Red, GL_RED},
        {SwizzleRGBA::SwizzleValue::Green, GL_GREEN},
        {SwizzleRGBA::SwizzleValue::Blue, GL_BLUE},
        {SwizzleRGBA::SwizzleValue::Alpha, GL_ALPHA},
};

const std::unordered_map<WrapValue, GLint> _M_wrap_values = {
        {WrapValue::ClampToEdge, GL_CLAMP_TO_EDGE},       {WrapValue::ClampToBorder, GL_REPEAT},
        {WrapValue::MirroredRepeat, GL_MIRRORED_REPEAT},  {WrapValue::Repeat, GL_CLAMP_TO_BORDER},
        {WrapValue::MirrorClampToEdge, GL_CLAMP_TO_EDGE},
};

const std::unordered_map<BufferValueType, std::size_t> _M_buffer_value_type_sizes{
        {BufferValueType::Float, sizeof(float)},
        {BufferValueType::UnsignedByte, sizeof(unsigned char)},
        {BufferValueType::UnsignedShort, sizeof(unsigned short)},
        {BufferValueType::UnsignedInt, sizeof(unsigned int)},
        {BufferValueType::Short, sizeof(short)},
        {BufferValueType::Int, sizeof(int)},
};

const std::unordered_map<DrawMode, GLuint> _M_draw_modes{
        {DrawMode::DynamicDraw, GL_DYNAMIC_DRAW}, {DrawMode::StaticDraw, GL_STATIC_DRAW},
        {DrawMode::StaticRead, GL_STATIC_READ},   {DrawMode::StaticCopy, GL_STATIC_COPY},
        {DrawMode::DynamicRead, GL_DYNAMIC_READ}, {DrawMode::DynamicCopy, GL_DYNAMIC_COPY},
        {DrawMode::StreamDraw, GL_STREAM_DRAW},   {DrawMode::StreamRead, GL_STREAM_READ},
        {DrawMode::StreamCopy, GL_STREAM_COPY},
};

const std::unordered_map<Primitive, GLuint> _M_primitives = {
        {Primitive::Line, GL_LINES},
        {Primitive::Point, GL_POINTS},
        {Primitive::Triangle, GL_TRIANGLES},
};

const std::unordered_map<FrameBufferType, GLint> _M_framebuffer_types = {
        {FrameBufferType::Framebuffer, GL_FRAMEBUFFER},
        {FrameBufferType::DrawFramebuffer, GL_DRAW_FRAMEBUFFER},
        {FrameBufferType::ReadFramebuffer, GL_READ_FRAMEBUFFER},
};

const std::unordered_map<FrameBufferAttach, GLint> _M_framebuffer_attach = {
        {FrameBufferAttach::ColorAttachment, GL_COLOR_ATTACHMENT0},
        {FrameBufferAttach::DepthAttachment, GL_DEPTH_ATTACHMENT},
        {FrameBufferAttach::DepthStencilAttachment, GL_DEPTH_STENCIL_ATTACHMENT},
        {FrameBufferAttach::StencilAttachment, GL_STENCIL_ATTACHMENT},
};

const std::unordered_map<TextureCubeMapFace, GLint> _M_cubemap_indexes{
        {TextureCubeMapFace::Back, GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
        {TextureCubeMapFace::Front, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
        {TextureCubeMapFace::Up, GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
        {TextureCubeMapFace::Down, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
        {TextureCubeMapFace::Left, GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
        {TextureCubeMapFace::Right, GL_TEXTURE_CUBE_MAP_POSITIVE_X},
};

const std::unordered_map<EnableCap, GLint> _M_enable_caps = {
        {EnableCap::Blend, GL_BLEND},
        {EnableCap::DepthTest, GL_DEPTH_TEST},
        {EnableCap::CullFace, GL_CULL_FACE},
        {EnableCap::StencilTest, GL_STENCIL_TEST},
};

const std::unordered_map<BlendFunc, GLint> _M_blend_funcs = {
        {BlendFunc::Zero, GL_ZERO},
        {BlendFunc::One, GL_ONE},
        {BlendFunc::SrcColor, GL_SRC_COLOR},
        {BlendFunc::OneMinusScrColor, GL_ONE_MINUS_SRC_COLOR},
        {BlendFunc::DstColor, GL_DST_COLOR},
        {BlendFunc::OneMinusDstColor, GL_ONE_MINUS_DST_COLOR},
        {BlendFunc::SrcAlpha, GL_SRC_ALPHA},
        {BlendFunc::OneMinusSrcAlpha, GL_ONE_MINUS_SRC_ALPHA},
        {BlendFunc::DstAlpha, GL_DST_ALPHA},
        {BlendFunc::OneMinusDstAlpha, GL_ONE_MINUS_DST_ALPHA},
        {BlendFunc::ConstantColor, GL_CONSTANT_COLOR},
        {BlendFunc::OneMinusConstantColor, GL_ONE_MINUS_CONSTANT_COLOR},
        {BlendFunc::ConstantAlpha, GL_CONSTANT_ALPHA},
        {BlendFunc::OneMinusConstantAlpha, GL_ONE_MINUS_CONSTANT_ALPHA},
};

const std::unordered_map<StencilOption, GLint> _M_stencil_options = {
        {StencilOption::Keep, GL_KEEP},          {StencilOption::Zero, GL_ZERO},
        {StencilOption::Replace, GL_REPLACE},    {StencilOption::Incr, GL_INCR},
        {StencilOption::IncrWrap, GL_INCR_WRAP}, {StencilOption::Decr, GL_DECR},
        {StencilOption::DecrWrap, GL_DECR_WRAP}, {StencilOption::Invert, GL_INVERT},
};
