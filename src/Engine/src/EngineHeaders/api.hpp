#pragma once

#include <Core/color.hpp>
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

class SDL_Window;
namespace Engine
{
    class Logger;
}

#ifdef ENGINE_API_IMPLEMENTATION
#define VIRTUAL_METHOD
#else
#define VIRTUAL_METHOD = 0
#endif

namespace Engine::GraphicApiInterface
{
    ENGINE_EXPORT struct ApiInterface {
        virtual ApiInterface& logger(Logger*&) VIRTUAL_METHOD;
        virtual void* init_window(SDL_Window*) VIRTUAL_METHOD;
        virtual ApiInterface& destroy_window() VIRTUAL_METHOD;
        virtual ApiInterface& destroy_object(ObjID&) VIRTUAL_METHOD;

        ///////////////// TEXTURE PART /////////////////
        virtual ApiInterface& create_texture(ObjID&, const TextureParams&) VIRTUAL_METHOD;
        virtual ApiInterface& bind_texture(const ObjID&, TextureBindIndex) VIRTUAL_METHOD;
        virtual const TextureParams* param_texture(const ObjID&) VIRTUAL_METHOD;
        virtual int_t base_level_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& base_level_texture(const ObjID&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& depth_stencil_mode_texture(const ObjID&, DepthStencilMode) VIRTUAL_METHOD;
        virtual DepthStencilMode depth_stencil_mode_texture(const ObjID&) VIRTUAL_METHOD;
        virtual CompareFunc compare_func_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& compare_func_texture(const ObjID&, CompareFunc) VIRTUAL_METHOD;
        virtual CompareMode compare_mode_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& compare_mode_texture(const ObjID&, CompareMode) VIRTUAL_METHOD;
        virtual TextureFilter min_filter_texture(const ObjID&) VIRTUAL_METHOD;
        virtual TextureFilter mag_filter_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& min_filter_texture(const ObjID&, TextureFilter) VIRTUAL_METHOD;
        virtual ApiInterface& mag_filter_texture(const ObjID&, TextureFilter) VIRTUAL_METHOD;
        virtual ApiInterface& min_lod_level_texture(const ObjID&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& max_lod_level_texture(const ObjID&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& max_mipmap_level_texture(const ObjID&, int_t) VIRTUAL_METHOD;
        virtual int_t min_lod_level_texture(const ObjID&) VIRTUAL_METHOD;
        virtual int_t max_lod_level_texture(const ObjID&) VIRTUAL_METHOD;
        virtual int_t max_mipmap_level_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& swizzle_texture(const ObjID&, const SwizzleRGBA&) VIRTUAL_METHOD;
        virtual SwizzleRGBA swizzle_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_s_texture(const ObjID&, const WrapValue&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_t_texture(const ObjID&, const WrapValue&) VIRTUAL_METHOD;
        virtual ApiInterface& wrap_r_texture(const ObjID&, const WrapValue&) VIRTUAL_METHOD;
        virtual WrapValue wrap_s_texture(const ObjID&) VIRTUAL_METHOD;
        virtual WrapValue wrap_t_texture(const ObjID&) VIRTUAL_METHOD;
        virtual WrapValue wrap_r_texture(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& copy_read_buffer_to_texture_2D(const ObjID&, const Size2D&, const Point2D&,
                                                             int_t) VIRTUAL_METHOD;
        virtual ApiInterface& texture_2D_update_from_current_read_buffer(const ObjID&, const Size2D&, const Offset2D&,
                                                                         const Point2D&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& gen_texture_2D(const ObjID&, const Size2D&, int_t, void*) VIRTUAL_METHOD;
        virtual ApiInterface& update_texture_2D(const ObjID&, const Size2D&, const Offset2D&, int_t,
                                                void*) VIRTUAL_METHOD;
        virtual ApiInterface& texture_size(const ObjID&, Size3D&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& generate_texture_mipmap(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& read_texture_2D_data(const ObjID&, std::vector<byte>& data, int_t) VIRTUAL_METHOD;
        virtual ObjID texture_id(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& cubemap_texture_attach_2d_texture(const ObjID&, const ObjID&, TextureCubeMapFace,
                                                                int_t) VIRTUAL_METHOD;
        virtual ApiInterface& cubemap_texture_attach_data(const ObjID&, TextureCubeMapFace, const Size2D&, int_t,
                                                          void*) VIRTUAL_METHOD;
        virtual ApiInterface& generate_mesh(ObjID&) VIRTUAL_METHOD;

        virtual ApiInterface& mesh_data(const ObjID&, size_t, DrawMode, void*) VIRTUAL_METHOD;
        virtual ApiInterface& draw_mesh(const ObjID&, Primitive, size_t, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& update_mesh_data(const ObjID&, size_t, size_t, void*) VIRTUAL_METHOD;
        virtual ApiInterface& mesh_indexes_array(const ObjID&, size_t, const IndexBufferComponent&, void*) VIRTUAL_METHOD;
        virtual ApiInterface& update_mesh_indexes_array(const ObjID&, size_t, size_t, void* data) VIRTUAL_METHOD;
        virtual ApiInterface& gen_framebuffer(ObjID&, FrameBufferType) VIRTUAL_METHOD;
        virtual ApiInterface& clear_frame_buffer(const ObjID&, BufferType) VIRTUAL_METHOD;
        virtual ApiInterface& bind_framebuffer(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& framebuffer_viewport(const Point2D&, const Size2D&) VIRTUAL_METHOD;
        virtual ApiInterface& attach_texture_to_framebuffer(const ObjID&, const ObjID&, FrameBufferAttach,
                                                            TextureAttachIndex, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& enable(Engine::EnableCap) VIRTUAL_METHOD;
        virtual ApiInterface& disable(Engine::EnableCap) VIRTUAL_METHOD;
        virtual ApiInterface& blend_func(Engine::BlendFunc, Engine::BlendFunc) VIRTUAL_METHOD;
        virtual ApiInterface& depth_func(Engine::DepthFunc) VIRTUAL_METHOD;
        virtual float line_rendering_width() VIRTUAL_METHOD;
        virtual ApiInterface& line_rendering_width(float) VIRTUAL_METHOD;
        virtual ApiInterface& create_shader(ObjID&, const ShaderParams&) VIRTUAL_METHOD;
        virtual ApiInterface& use_shader(const ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, float) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, int_t) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, const glm::mat3&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, const glm::mat4&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, const glm::vec2&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, const glm::vec3&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, const glm::vec4&) VIRTUAL_METHOD;
        virtual ApiInterface& shader_value(const ObjID&, const String&, void*) VIRTUAL_METHOD;
        virtual ApiInterface& depth_mask(bool) VIRTUAL_METHOD;
        virtual ApiInterface& stencil_mask(byte) VIRTUAL_METHOD;
        virtual ApiInterface& stencil_func(Engine::CompareFunc, int_t, byte) VIRTUAL_METHOD;
        virtual ApiInterface& stencil_option(Engine::StencilOption, Engine::StencilOption,
                                             Engine::StencilOption) VIRTUAL_METHOD;
        virtual ApiInterface& create_ssbo(ObjID&) VIRTUAL_METHOD;
        virtual ApiInterface& bind_ssbo(const ObjID&, int_t slot) VIRTUAL_METHOD;
        virtual ApiInterface& ssbo_data(const ObjID&, void*, size_t, BufferUsage) VIRTUAL_METHOD;
        virtual ApiInterface& update_ssbo_data(const ObjID&, void*, size_t, size_t) VIRTUAL_METHOD;
        virtual ApiInterface& swap_buffer(SDL_Window* window) VIRTUAL_METHOD;
        virtual ApiInterface& swap_interval(int_t interval) VIRTUAL_METHOD;
        virtual ApiInterface& clear_color(const Color& color) VIRTUAL_METHOD;
        virtual ApiInterface& on_window_size_changed() VIRTUAL_METHOD;
        virtual ApiInterface& begin_render() VIRTUAL_METHOD;
        virtual ApiInterface& end_render() VIRTUAL_METHOD;
        virtual ApiInterface& begin_render_pass() VIRTUAL_METHOD;
        virtual ApiInterface& end_render_pass() VIRTUAL_METHOD;
        virtual ApiInterface& wait_idle() VIRTUAL_METHOD;


        virtual ~ApiInterface();
    };

#undef VIRTUAL_METHOD
}// namespace Engine::GraphicApiInterface
