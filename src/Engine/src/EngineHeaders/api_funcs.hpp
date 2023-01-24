#pragma once
#include <Core/engine_types.hpp>
#include <Core/logger.hpp>
#include <api_function.hpp>


namespace Engine
{
    extern ApiFunction<bool, std::vector<int>> api_init;
    extern ApiFunction<void> api_terminate;
    extern ApiFunction<void, Logger*&> set_logger;
    extern ApiFunction<void*, class SDL_Window*> api_init_window;
    extern ApiFunction<void> api_destroy_window;
    extern ApiFunction<void, ObjID&, const TextureParams&> create_texture;
    extern ApiFunction<void, ObjID&> destroy_object;
    extern ApiFunction<void, const ObjID&, unsigned int> bind_texture;
    extern ApiFunction<const TextureParams*, const ObjID&> get_param_texture;
    extern ApiFunction<void, const ObjID&, ObjID&> link_object;
    extern ApiFunction<int, const ObjID&> get_base_level_texture;
    extern ApiFunction<void, const ObjID&, int> set_base_level_texture;
    extern ApiFunction<void, const ObjID&, DepthStencilMode> set_depth_stencil_mode_texture;
    extern ApiFunction<DepthStencilMode, const ObjID&> get_depth_stencil_mode_texture;
    extern ApiFunction<CompareFunc, const ObjID&> get_compare_func_texture;
    extern ApiFunction<void, const ObjID&, CompareFunc> set_compare_func_texture;
    extern ApiFunction<CompareMode, const ObjID&> get_compare_mode_texture;
    extern ApiFunction<void, const ObjID&, CompareMode> set_compare_mode_texture;
    extern ApiFunction<TextureFilter, const ObjID&> get_min_filter_texture;
    extern ApiFunction<TextureFilter, const ObjID&> get_mag_filter_texture;
    extern ApiFunction<void, const ObjID&, TextureFilter> set_min_filter_texture;
    extern ApiFunction<void, const ObjID&, TextureFilter> set_mag_filter_texture;
    extern ApiFunction<void, const ObjID&, int> set_min_lod_level_texture;
    extern ApiFunction<void, const ObjID&, int> set_max_lod_level_texture;
    extern ApiFunction<void, const ObjID&, int> set_max_mipmap_level_texture;
    extern ApiFunction<int, const ObjID&> get_min_lod_level_texture;
    extern ApiFunction<int, const ObjID&> get_max_lod_level_texture;
    extern ApiFunction<int, const ObjID&> get_max_mipmap_level_texture;
    extern ApiFunction<void, const ObjID&, const SwizzleRGBA&> set_swizzle_texture;
    extern ApiFunction<SwizzleRGBA, const ObjID&> get_swizzle_texture;
    extern ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_s_texture;
    extern ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_t_texture;
    extern ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_r_texture;
    extern ApiFunction<WrapValue, const ObjID&> get_wrap_s_texture;
    extern ApiFunction<WrapValue, const ObjID&> get_wrap_t_texture;
    extern ApiFunction<WrapValue, const ObjID&> get_wrap_r_texture;
    extern ApiFunction<void, const ObjID&, const Size2D&, const Point2D&, int> copy_read_buffer_to_texture_2D;
    extern ApiFunction<void, const ObjID&, const Size2D&, const Offset2D&, const Point2D&, int>
            texture_2D_update_from_current_read_buffer;
    extern ApiFunction<void, const ObjID&, const Size2D&, int, void*> gen_texture_2D;
    extern ApiFunction<void, const ObjID&, const Size2D&, const Offset2D&, int, void*> update_texture_2D;
    extern ApiFunction<void, const ObjID&, Size3D&, int> get_size_texture;
    extern ApiFunction<void, const ObjID&> generate_texture_mipmap;
    extern ApiFunction<void, const ObjID&, std::vector<byte>&, int> read_texture_2D_data;
    extern ApiFunction<ObjID, const ObjID&> texture_id;
    extern ApiFunction<void, const ObjID&, const ObjID&, TextureCubeMapFace, int> cubemap_texture_attach_2d_texture;
    extern ApiFunction<void, const ObjID&, TextureCubeMapFace, const Size2D&, int, void*> cubemap_texture_attach_data;
    extern ApiFunction<void, ObjID&> generate_mesh;
    extern ApiFunction<void, const ObjID&, std::size_t, DrawMode, void*> set_mesh_data;
    extern ApiFunction<void, const ObjID&, const MeshInfo&> update_mesh_attributes;
    extern ApiFunction<void, const ObjID&, Primitive, std::size_t, std::size_t> draw_mesh;
    extern ApiFunction<void, const ObjID&, std::size_t, std::size_t, void*> update_mesh_data;
    extern ApiFunction<void, const ObjID&, const MeshInfo&, std::size_t, const BufferValueType&, void*> set_mesh_indexes_array;
    extern ApiFunction<void, ObjID&, FrameBufferType> gen_framebuffer;
    extern ApiFunction<void, const ObjID&, BufferType> clear_frame_buffer;
    extern ApiFunction<void, const ObjID&> bind_framebuffer;
    extern ApiFunction<void, const ObjID&, const Point2D&, const Size2D&> set_framebuffer_viewport;
    extern ApiFunction<void, const ObjID&, const ObjID&, FrameBufferAttach, unsigned int, int> attach_texture_to_framebuffer;
    extern ApiFunction<void, Engine::EnableCap> api_enable;
    extern ApiFunction<void, Engine::EnableCap> api_disable;
    extern ApiFunction<void, Engine::BlendFunc, Engine::BlendFunc> set_blend_func;
    extern ApiFunction<void, Engine::CompareFunc> set_depth_func;
    extern ApiFunction<float> get_current_line_rendering_width;
    extern ApiFunction<void, float> set_line_rendering_width;
    extern ApiFunction<void, ObjID&, const ShaderParams&> create_shader;
    extern ApiFunction<void, const ObjID&> use_shader;
    extern ApiFunction<void, const ObjID&, const std::string&, float> set_shader_float_value;
    extern ApiFunction<void, const ObjID&, const std::string&, int> set_shader_int_value;
    extern ApiFunction<void, const ObjID&, const std::string&, const glm::mat3&> set_shader_mat3_float_value;
    extern ApiFunction<void, const ObjID&, const std::string&, const glm::mat4&> set_shader_mat4_float_value;
    extern ApiFunction<void, const ObjID&, const std::string&, const glm::vec2&> set_shader_vec2_float_value;
    extern ApiFunction<void, const ObjID&, const std::string&, const glm::vec3&> set_shader_vec3_float_value;
    extern ApiFunction<void, const ObjID&, const std::string&, const glm::vec4&> set_shader_vec4_float_value;
    extern ApiFunction<void, bool> set_depth_mask;
    extern ApiFunction<void, byte> set_stencil_mask;
    extern ApiFunction<void, Engine::CompareFunc, int, byte> set_stencil_func;
    extern ApiFunction<void, Engine::StencilOption, Engine::StencilOption, Engine::StencilOption> set_stencil_option;
    extern ApiFunction<void, ObjID&> create_ssbo;
    extern ApiFunction<void, const ObjID&> bind_ssbo;
    extern ApiFunction<void, const ObjID&, void*, std::size_t, BufferUsage> set_ssbo_data;
    extern ApiFunction<void, const ObjID&, void*, std::size_t, std::size_t> update_ssbo_data;
    //    extern bool (*api_init)(std::vector<int> params);
    //    extern void (*api_terminate)();
    //    extern void (*set_logger)(Logger*& logger);
    //    extern void* (*api_init_window)(class SDL_Window* window);
    //    extern void (*api_destroy_window)();

    //    extern void (*set_logger)(Logger*& logger);
    //    extern void (*create_texture)(ObjID& ID, const TextureParams& params);

    //    extern void (*destroy_object)(ObjID& ID);

    //    extern void (*bind_texture)(const ObjID& ID, unsigned int num);
    //    extern const TextureParams* (*get_param_texture)(const ObjID& ID);
    //    extern void (*link_object)(const ObjID& src_ID, ObjID& dest_ID);
    //    extern int (*get_base_level_texture)(const ObjID& ID);
    //    extern void (*set_base_level_texture)(const ObjID& ID, int level);
    //    extern void (*set_depth_stencil_mode_texture)(const ObjID& ID, DepthStencilMode mode);
    //    extern DepthStencilMode (*get_depth_stencil_mode_texture)(const ObjID& ID);

    //    extern CompareFunc (*get_compare_func_texture)(const ObjID& ID);
    //    extern void (*set_compare_func_texture)(const ObjID& ID, CompareFunc func);

    //    extern CompareMode (*get_compare_mode_texture)(const ObjID& ID);
    //    extern void (*set_compare_mode_texture)(const ObjID& ID, CompareMode mode);

    //    extern TextureFilter (*get_min_filter_texture)(const ObjID& ID);
    //    extern TextureFilter (*get_mag_filter_texture)(const ObjID& ID);
    //    extern void (*set_min_filter_texture)(const ObjID& ID, TextureFilter filter);
    //    extern void (*set_mag_filter_texture)(const ObjID& ID, TextureFilter filter);

    //    extern void (*set_min_lod_level_texture)(const ObjID& ID, int level);
    //    extern void (*set_max_lod_level_texture)(const ObjID& ID, int level);
    //    extern void (*set_max_mipmap_level_texture)(const ObjID& ID, int level);
    //    extern int (*get_min_lod_level_texture)(const ObjID& ID);
    //    extern int (*get_max_lod_level_texture)(const ObjID& ID);
    //    extern int (*get_max_mipmap_level_texture)(const ObjID& ID);

    //    extern void (*set_swizzle_texture)(const ObjID& ID, const SwizzleRGBA& value);
    //    extern SwizzleRGBA (*get_swizzle_texture)(const ObjID& ID);
    //    extern void (*set_wrap_s_texture)(const ObjID& ID, const WrapValue& wrap);
    //    extern void (*set_wrap_t_texture)(const ObjID& ID, const WrapValue& wrap);
    //    extern void (*set_wrap_r_texture)(const ObjID& ID, const WrapValue& wrap);
    //    extern WrapValue (*get_wrap_s_texture)(const ObjID& ID);
    //    extern WrapValue (*get_wrap_t_texture)(const ObjID& ID);
    //    extern WrapValue (*get_wrap_r_texture)(const ObjID& ID);
    //    extern void (*copy_read_buffer_to_texture_2D)(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap);
    //    extern void (*gen_texture_2D)(const ObjID& ID, const Size2D& size, int level, void* data);
    //    extern void (*update_texture_2D)(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data);
    //    extern void (*generate_texture_mipmap)(const ObjID& _ID);
    //    extern void (*get_size_texture)(const ObjID& ID, Size3D& size, int level);
    //    extern void (*texture_2D_update_from_current_read_buffer)(const ObjID& ID, const Size2D& size, const Offset2D& offset,
    //                                                              const Point2D& pos, int mipmap);
    //    extern void (*read_texture_2D_data)(const ObjID& ID, std::vector<byte>& data, int level);
    //    extern ObjID (*texture_id)(const ObjID& ID);
    //    extern void (*cubemap_texture_attach_2d_texture)(const ObjID& ID, const ObjID& texture, TextureCubeMapFace index,
    //                                                     int level);
    //    extern void (*cubemap_texture_attach_data)(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
    //                                               void* data);

    //    ////////////////////////////////// MESH //////////////////////////////////
    //    extern void (*generate_mesh)(ObjID& ID);
    //    extern void (*set_mesh_data)(const ObjID& ID, std::size_t buffer_len, BufferUsage mode, void* data);
    //    extern void (*update_mesh_attributes)(const ObjID& ID, const MeshInfo& info);
    //    extern void (*draw_mesh)(const ObjID& ID, Primitive primitive, std::size_t vertices, std::size_t offset);
    //    extern void (*update_mesh_data)(const ObjID& ID, std::size_t offset, std::size_t size, void* data);

    //    extern void (*gen_framebuffer)(ObjID& ID, FrameBufferType type);
    //    extern void (*clear_frame_buffer)(const ObjID& ID, BufferType type);
    //    extern void (*bind_framebuffer)(const ObjID& ID);
    //    extern void (*set_framebuffer_viewport)(const ObjID& ID, const Point2D& pos, const Size2D& size);
    //    extern void (*attach_texture_to_framebuffer)(const ObjID& ID, const ObjID& texture, FrameBufferAttach attach,
    //                                                 unsigned int num, int level);

    //    extern void (*api_enable)(Engine::EnableCap cap);
    //    extern void (*api_disable)(Engine::EnableCap cap);
    //    extern void (*set_blend_func)(Engine::BlendFunc, Engine::BlendFunc);
    //    extern void (*set_depth_func)(Engine::CompareFunc);
    //    extern float (*get_current_line_rendering_width)();
    //    extern void (*set_line_rendering_width)(float value);
    //    extern void (*set_mesh_indexes_array)(const ObjID& ID, const MeshInfo& info, std::size_t bytes,
    //                                          const BufferValueType& data_type, void* data);

    //    // Shader system

    //    extern void (*create_shader)(ObjID& ID, const ShaderParams& params);
    //    extern void (*use_shader)(const ObjID& ID);
    //    extern void (*set_shader_float_value)(const ObjID& ID, const std::string& name, float value);
    //    extern void (*set_shader_int_value)(const ObjID& ID, const std::string& name, int value);
    //    extern void (*set_shader_mat3_float_value)(const ObjID& ID, const std::string& name, const glm::mat3& matrix);
    //    extern void (*set_shader_mat4_float_value)(const ObjID& ID, const std::string& name, const glm::mat4& matrix);
    //    extern void (*set_shader_vec2_float_value)(const ObjID& ID, const std::string& name, const glm::vec2& matrix);
    //    extern void (*set_shader_vec3_float_value)(const ObjID& ID, const std::string& name, const glm::vec3& matrix);
    //    extern void (*set_shader_vec4_float_value)(const ObjID& ID, const std::string& name, const glm::vec4& matrix);

    //    extern void (*set_depth_mask)(bool mask);
    //    extern void (*set_stencil_mask)(byte mask);
    //    extern void (*set_stencil_func)(Engine::CompareFunc func, int ref, byte mask);
    //    extern void (*set_stencil_option)(Engine::StencilOption stencil_fail, Engine::StencilOption depth_fail,
    //                                    Engine::StencilOption pass);


    //     ////////////////////////////////// SSBO  //////////////////////////////////

    //    extern void (*create_ssbo)(ObjID& ID);
    //    extern void (*bind_ssbo)(const ObjID& ID);
    //    extern void (*set_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, BufferUsage mode);
    //    extern void (*update_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, std::size_t offset);

}// namespace Engine
