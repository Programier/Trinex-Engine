#pragma once
#include <unordered_map>
#include <Core/engine_types.hpp>
#include <opengl_headers.hpp>

using namespace Engine;

extern const std::unordered_map<TextureType, GLuint> _M_types;
extern const std::unordered_map<PixelFormat, GLuint> _M_pixel_formats;
extern const std::unordered_map<BufferValueType, GLuint> _M_buffer_value_types;
extern const std::unordered_map<CompareFunc, GLuint> _M_compare_funcs;
extern const std::unordered_map<GLint, CompareFunc> _M_revert_compare_funcs;
extern const std::unordered_map<CompareMode, GLuint> _M_compare_modes;
extern const std::unordered_map<TextureFilter, GLuint> _M_texture_filters;
extern const std::unordered_map<GLint, TextureFilter> _M_reverse_texture_filters;
extern const std::unordered_map<SwizzleRGBA::SwizzleValue, GLint> _M_swizzle_values;
extern const std::unordered_map<WrapValue, GLint> _M_wrap_values;
extern const std::unordered_map<BufferValueType, std::size_t> _M_buffer_value_type_sizes;
extern const std::unordered_map<DrawMode, GLuint> _M_draw_modes;
extern const std::unordered_map<Primitive, GLuint> _M_primitives;
extern const std::unordered_map<FrameBufferType, GLint> _M_framebuffer_types;
extern const std::unordered_map<FrameBufferAttach, GLint> _M_framebuffer_attach;
extern const std::unordered_map<TextureCubeMapFace, GLint> _M_cubemap_indexes;
extern const std::unordered_map<EnableCap, GLint> _M_enable_caps;
extern const std::unordered_map<BlendFunc, GLint> _M_blend_funcs;
extern const std::unordered_map<StencilOption, GLint> _M_stencil_options;
