#include <opengl_types.hpp>


const std::unordered_map<TextureType, GLuint> _M_types = {
        {TextureType::Texture_2D, GL_TEXTURE_2D},
        {TextureType::Texture_2D_Array, GL_TEXTURE_2D_ARRAY},
        {TextureType::Texture_2D_MultiSample, GL_TEXTURE_2D_MULTISAMPLE},
        {TextureType::Texture_2D_MultiSample_Array, GL_TEXTURE_2D_MULTISAMPLE_ARRAY},
        {TextureType::Texture_3D, GL_TEXTURE_3D},
        {TextureType::Texture_Cube_Map, GL_TEXTURE_CUBE_MAP},
        {TextureType::Texture_Buffer, GL_TEXTURE_BUFFER},
};

const std::unordered_map<PixelFormat, GLuint> _M_pixel_formats = {
        {PixelFormat::RGB, GL_RGB},     {PixelFormat::RGBA, GL_RGBA},   {PixelFormat::DEPTH, GL_DEPTH_COMPONENT},
        {PixelFormat::RED, GL_RED},     {PixelFormat::GREEN, GL_GREEN}, {PixelFormat::BLUE, GL_BLUE},
        {PixelFormat::ALPHA, GL_ALPHA},
};


const std::unordered_map<BufferValueType, GLuint> _M_buffer_value_types = {
        {BufferValueType::FLOAT, GL_FLOAT},
        {BufferValueType::UNSIGNED_BYTE, GL_UNSIGNED_BYTE},
        {BufferValueType::UNSIGNED_SHORT, GL_UNSIGNED_SHORT},
        {BufferValueType::UNSIGNED_INT, GL_UNSIGNED_INT},
        {BufferValueType::SHORT, GL_SHORT},
        {BufferValueType::INT, GL_INT},
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
        {CompareMode::NONE, GL_NONE},
        {CompareMode::REF_TO_TEXTURE, GL_COMPARE_REF_TO_TEXTURE},
};

const std::unordered_map<TextureFilter, GLuint> _M_texture_filters = {
        {TextureFilter::NEAREST, GL_NEAREST},
        {TextureFilter::LINEAR, GL_LINEAR},
        {TextureFilter::NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST},
        {TextureFilter::NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR},
        {TextureFilter::LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST},
        {TextureFilter::LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR},
};


const std::unordered_map<GLint, TextureFilter> _M_reverse_texture_filters = {
        {GL_NEAREST, TextureFilter::NEAREST},
        {GL_LINEAR, TextureFilter::LINEAR},
        {GL_NEAREST_MIPMAP_NEAREST, TextureFilter::NEAREST_MIPMAP_NEAREST},
        {GL_NEAREST_MIPMAP_LINEAR, TextureFilter::NEAREST_MIPMAP_LINEAR},
        {GL_LINEAR_MIPMAP_NEAREST, TextureFilter::LINEAR_MIPMAP_NEAREST},
        {GL_LINEAR_MIPMAP_LINEAR, TextureFilter::LINEAR_MIPMAP_LINEAR},
};

const std::unordered_map<SwizzleRGBA::SwizzleValue, GLint> _M_swizzle_values = {
        {SwizzleRGBA::SwizzleValue::RED, GL_RED},
        {SwizzleRGBA::SwizzleValue::GREEN, GL_GREEN},
        {SwizzleRGBA::SwizzleValue::BLUE, GL_BLUE},
        {SwizzleRGBA::SwizzleValue::ALPHA, GL_ALPHA},
};

const std::unordered_map<WrapValue, GLint> _M_wrap_values = {
        {WrapValue::CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},        {WrapValue::CLAMP_TO_BORDER, GL_REPEAT},
        {WrapValue::MIRRORED_REPEAT, GL_MIRRORED_REPEAT},    {WrapValue::REPEAT, GL_CLAMP_TO_BORDER},
        {WrapValue::MIRROR_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
};

const std::unordered_map<BufferValueType, std::size_t> _M_buffer_value_type_sizes{
        {BufferValueType::FLOAT, sizeof(float)},
        {BufferValueType::UNSIGNED_BYTE, sizeof(unsigned char)},
        {BufferValueType::UNSIGNED_SHORT, sizeof(unsigned short)},
        {BufferValueType::UNSIGNED_INT, sizeof(unsigned int)},
        {BufferValueType::SHORT, sizeof(short)},
        {BufferValueType::INT, sizeof(int)},
};

const std::unordered_map<DrawMode, GLuint> _M_draw_modes{
        {DrawMode::DYNAMIC_DRAW, GL_DYNAMIC_DRAW},
        {DrawMode::STATIC_DRAW, GL_STATIC_DRAW},
};

const std::unordered_map<Primitive, GLuint> _M_primitives = {
        {Primitive::LINE, GL_LINES},
        {Primitive::POINT, GL_POINTS},
        {Primitive::TRIANGLE, GL_TRIANGLES},
};

const std::unordered_map<FrameBufferType, GLint> _M_framebuffer_types = {
        {FrameBufferType::FRAMEBUFFER, GL_FRAMEBUFFER},
        {FrameBufferType::DRAW_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER},
        {FrameBufferType::READ_FRAMEBUFFER, GL_READ_FRAMEBUFFER},
};

const std::unordered_map<FrameBufferAttach, GLint> _M_framebuffer_attach = {
        {FrameBufferAttach::COLOR_ATTACHMENT, GL_COLOR_ATTACHMENT0},
        {FrameBufferAttach::DEPTH_ATTACHMENT, GL_DEPTH_ATTACHMENT},
        {FrameBufferAttach::DEPTH_STENCIL_ATTACHMENT, GL_DEPTH_STENCIL_ATTACHMENT},
        {FrameBufferAttach::STENCIL_ATTACHMENT, GL_STENCIL_ATTACHMENT},
};

const std::unordered_map<TextureCubeMapFace, GLint> _M_cubemap_indexes{
        {TextureCubeMapFace::BACK, GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
        {TextureCubeMapFace::FRONT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
        {TextureCubeMapFace::UP, GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
        {TextureCubeMapFace::DOWN, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
        {TextureCubeMapFace::LEFT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
        {TextureCubeMapFace::RIGHT, GL_TEXTURE_CUBE_MAP_POSITIVE_X},
};

const std::unordered_map<EnableCap, GLint> _M_enable_caps = {
        {EnableCap::Blend, GL_BLEND},
        {EnableCap::DepthTest, GL_DEPTH_TEST},
        {EnableCap::CullFace, GL_CULL_FACE},
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
