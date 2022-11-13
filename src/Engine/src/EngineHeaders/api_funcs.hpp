#pragma once
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>


namespace Engine
{
    extern bool (*api_init)(std::vector<int> params);
    extern void (*api_terminate)();
    extern void (*set_logger)(Logger*& logger);
    extern void* (*api_init_window)(class SDL_Window* window);
    extern void (*api_destroy_window)();

    extern void (*set_logger)(Logger*& logger);
    extern void (*create_texture)(ObjID& ID, const TextureParams& params);

    extern void (*destroy_object)(ObjID& ID);

    extern void (*bind_texture)(const ObjID& ID, unsigned int num);
    extern const TextureParams* (*get_param_texture)(const ObjID& ID);
    extern void (*link_object)(const ObjID& src_ID, ObjID& dest_ID);
    extern int (*get_base_level_texture)(const ObjID& ID);
    extern void (*set_base_level_texture)(const ObjID& ID, int level);
    extern void (*set_depth_stencil_mode_texture)(const ObjID& ID, DepthStencilMode mode);
    extern DepthStencilMode (*get_depth_stencil_mode_texture)(const ObjID& ID);

    extern CompareFunc (*get_compare_func_texture)(const ObjID& ID);
    extern void (*set_compare_func_texture)(const ObjID& ID, CompareFunc func);

    extern CompareMode (*get_compare_mode_texture)(const ObjID& ID);
    extern void (*set_compare_mode_texture)(const ObjID& ID, CompareMode mode);

    extern TextureFilter (*get_min_filter_texture)(const ObjID& ID);
    extern TextureFilter (*get_mag_filter_texture)(const ObjID& ID);
    extern void (*set_min_filter_texture)(const ObjID& ID, TextureFilter filter);
    extern void (*set_mag_filter_texture)(const ObjID& ID, TextureFilter filter);

    extern void (*set_min_lod_level_texture)(const ObjID& ID, int level);
    extern void (*set_max_lod_level_texture)(const ObjID& ID, int level);
    extern void (*set_max_mipmap_level_texture)(const ObjID& ID, int level);
    extern int (*get_min_lod_level_texture)(const ObjID& ID);
    extern int (*get_max_lod_level_texture)(const ObjID& ID);
    extern int (*get_max_mipmap_level_texture)(const ObjID& ID);

    extern void (*set_swizzle_texture)(const ObjID& ID, const SwizzleRGBA& value);
    extern SwizzleRGBA (*get_swizzle_texture)(const ObjID& ID);
    extern void (*set_wrap_s_texture)(const ObjID& ID, const WrapValue& wrap);
    extern void (*set_wrap_t_texture)(const ObjID& ID, const WrapValue& wrap);
    extern void (*set_wrap_r_texture)(const ObjID& ID, const WrapValue& wrap);
    extern WrapValue (*get_wrap_s_texture)(const ObjID& ID);
    extern WrapValue (*get_wrap_t_texture)(const ObjID& ID);
    extern WrapValue (*get_wrap_r_texture)(const ObjID& ID);
    extern void (*copy_read_buffer_to_texture_2D)(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap);
    extern void (*gen_texture_2D)(const ObjID& ID, const Size2D& size, int level, void* data);
    extern void (*update_texture_2D)(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data);
    extern void (*generate_texture_mipmap)(const ObjID& _ID);
    extern void (*get_size_texture)(const ObjID& ID, Size3D& size, int level);
    extern void (*texture_2D_update_from_current_read_buffer)(const ObjID& ID, const Size2D& size, const Offset2D& offset,
                                                              const Point2D& pos, int mipmap);
    extern void (*read_texture_2D_data)(const ObjID& ID, std::vector<byte>& data, int level);
    extern ObjID (*texture_id)(const ObjID& ID);
    extern void (*cubemap_texture_attach_2d_texture)(const ObjID& ID, const ObjID& texture, TextureCubeMapFace index,
                                                     int level);
    extern void (*cubemap_texture_attach_data)(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
                                               void* data);

    ////////////////////////////////// MESH //////////////////////////////////
    extern void (*generate_mesh)(ObjID& ID);
    extern void (*set_mesh_data)(const ObjID& ID, MeshInfo& info, void* data);
    extern void (*update_mesh_attributes)(const ObjID& ID, MeshInfo& info);
    extern void (*draw_mesh)(const ObjID& ID, Primitive primitive, std::size_t vertices, unsigned int start_index);
    extern void (*update_mesh_date)(const ObjID& ID, std::size_t offset, std::size_t size, void* data);

    extern void (*gen_framebuffer)(ObjID& ID, FrameBufferType type);
    extern void (*clear_frame_buffer)(const ObjID& ID, BufferType type);
    extern void (*bind_framebuffer)(const ObjID& ID);
    extern void (*set_framebuffer_viewport)(const ObjID& ID, const Point2D& pos, const Size2D& size);
    extern void (*attach_texture_to_framebuffer)(const ObjID& ID, const ObjID& texture, FrameBufferAttach attach,
                                                 unsigned int num, int level);

    extern void (*api_enable)(Engine::EnableCap cap);
    extern void (*api_disable)(Engine::EnableCap cap);
    extern void (*set_blend_func)(Engine::BlendFunc, Engine::BlendFunc);
    extern void (*set_depth_func)(Engine::CompareFunc);
    extern float (*get_current_line_rendering_width)();
    extern void (*set_line_rendering_width)(float value);

    // Shader system

    extern void (*create_shader)(ObjID& ID, const ShaderParams& params);
    extern void (*use_shader)(const ObjID& ID);
    extern void (*set_shader_float_value)(const ObjID& ID, const std::string& name, float value);
    extern void (*set_shader_int_value)(const ObjID& ID, const std::string& name, int value);
    extern void (*set_shader_mat3_float_value)(const ObjID& ID, const std::string& name, const glm::mat3& matrix);
    extern void (*set_shader_mat4_float_value)(const ObjID& ID, const std::string& name, const glm::mat4& matrix);
    extern void (*set_shader_vec2_float_value)(const ObjID& ID, const std::string& name, const glm::vec2& matrix);
    extern void (*set_shader_vec3_float_value)(const ObjID& ID, const std::string& name, const glm::vec3& matrix);
    extern void (*set_shader_vec4_float_value)(const ObjID& ID, const std::string& name, const glm::vec4& matrix);

}// namespace Engine
