#include <vulkan_api.hpp>


//bool (*api_init)(std::vector<int> params) = nullptr;
//void (*api_terminate)() = nullptr;
//void (*set_logger)(Logger*& logger) = nullptr;
//void* (*api_init_window)(class SDL_Window* window) = nullptr;
//void (*api_destroy_window)() = nullptr;

////////////////////////// TEXTURE 2D ////////////////////////
//void (*create_texture)(ObjID& ID, const TextureParams& params) = nullptr;
//void (*destroy_object)(ObjID& ID) = nullptr;
//void (*bind_texture)(const ObjID& ID, unsigned int num) = nullptr;
//const TextureParams* (*get_param_texture)(const ObjID& ID) = nullptr;
//void (*link_object)(const ObjID& src_ID, ObjID& dest_ID) = nullptr;
//int (*get_base_level_texture)(const ObjID& ID) = nullptr;
//void (*set_base_level_texture)(const ObjID& ID, int level) = nullptr;
//void (*set_depth_stencil_mode_texture)(const ObjID& ID, DepthStencilMode mode) = nullptr;
//DepthStencilMode (*get_depth_stencil_mode_texture)(const ObjID& ID) = nullptr;
//CompareFunc (*get_compare_func_texture)(const ObjID& ID) = nullptr;
//void (*set_compare_func_texture)(const ObjID& ID, CompareFunc func) = nullptr;
//CompareMode (*get_compare_mode_texture)(const ObjID& ID) = nullptr;
//void (*set_compare_mode_texture)(const ObjID& ID, CompareMode mode) = nullptr;
//TextureFilter (*get_min_filter_texture)(const ObjID& ID) = nullptr;
//TextureFilter (*get_mag_filter_texture)(const ObjID& ID) = nullptr;
//void (*set_min_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
//void (*set_mag_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
//void (*set_min_lod_level_texture)(const ObjID& ID, int level) = nullptr;
//void (*set_max_lod_level_texture)(const ObjID& ID, int level) = nullptr;
//void (*set_max_mipmap_level_texture)(const ObjID& ID, int level) = nullptr;
//int (*get_min_lod_level_texture)(const ObjID& ID) = nullptr;
//int (*get_max_lod_level_texture)(const ObjID& ID) = nullptr;
//int (*get_max_mipmap_level_texture)(const ObjID& ID) = nullptr;
//void (*set_swizzle_texture)(const ObjID& ID, const SwizzleRGBA& value) = nullptr;
//SwizzleRGBA (*get_swizzle_texture)(const ObjID& ID) = nullptr;
//void (*set_wrap_s_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
//void (*set_wrap_t_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
//void (*set_wrap_r_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
//WrapValue (*get_wrap_s_texture)(const ObjID& ID) = nullptr;
//WrapValue (*get_wrap_t_texture)(const ObjID& ID) = nullptr;
//WrapValue (*get_wrap_r_texture)(const ObjID& ID) = nullptr;
//void (*copy_read_buffer_to_texture_2D)(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap) = nullptr;
//void (*texture_2D_update_from_current_read_buffer)(const ObjID& ID, const Size2D& size, const Offset2D& offset,
//                                                   const Point2D& pos, int mipmap) = nullptr;
//void (*gen_texture_2D)(const ObjID& ID, const Size2D& size, int level, void* data) = nullptr;
//void (*update_texture_2D)(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data) = nullptr;
//void (*get_size_texture)(const ObjID& ID, Size3D& size, int level) = nullptr;
//void (*generate_texture_mipmap)(const ObjID& _ID) = nullptr;
//void (*read_texture_2D_data)(const ObjID& ID, std::vector<byte>& data, int level) = nullptr;
//ObjID (*texture_id)(const ObjID& ID) = nullptr;
//void (*cubemap_texture_attach_2d_texture)(const ObjID& ID, const ObjID& texture, TextureCubeMapFace index,
//                                          int level) = nullptr;
//void (*cubemap_texture_attach_data)(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
//                                    void* data) = nullptr;

//void (*generate_mesh)(ObjID& ID) = nullptr;
//void (*set_mesh_data)(const ObjID& ID, std::size_t buffer_len, DrawMode mode, void* data) = nullptr;
//void (*update_mesh_attributes)(const ObjID& ID, const MeshInfo& info) = nullptr;
//void (*draw_mesh)(const ObjID& ID, Primitive primitive, std::size_t vertices, std::size_t offset) = nullptr;
//void (*update_mesh_data)(const ObjID& ID, std::size_t offset, std::size_t size, void* data) = nullptr;
//void (*set_mesh_indexes_array)(const ObjID& ID, const MeshInfo& info, std::size_t bytes,
//                               const BufferValueType& data_type, void*) = nullptr;

//void (*gen_framebuffer)(ObjID& ID, FrameBufferType type) = nullptr;
//void (*clear_frame_buffer)(const ObjID& _M_ID, BufferType type) = nullptr;
//void (*bind_framebuffer)(const ObjID& ID) = nullptr;
//void (*set_framebuffer_viewport)(const ObjID& ID, const Point2D& pos, const Size2D& size) = nullptr;
//void (*attach_texture_to_framebuffer)(const ObjID& ID, const ObjID& texture, FrameBufferAttach attach, unsigned int num,
//                                      int level) = nullptr;

//void (*api_enable)(Engine::EnableCap cap) = nullptr;
//void (*api_disable)(Engine::EnableCap cap) = nullptr;
//void (*set_blend_func)(Engine::BlendFunc, Engine::BlendFunc) = nullptr;
//void (*set_depth_func)(Engine::CompareFunc func) = nullptr;
//float (*get_current_line_rendering_width)() = nullptr;
//void (*set_line_rendering_width)(float value) = nullptr;

//// Shader system
//void (*create_shader)(ObjID& ID, const ShaderParams& params) = nullptr;
//void (*use_shader)(const ObjID& ID) = nullptr;
//void (*set_shader_float_value)(const ObjID& ID, const std::string& name, float value) = nullptr;
//void (*set_shader_int_value)(const ObjID& ID, const std::string& name, int value) = nullptr;
//void (*set_shader_mat3_float_value)(const ObjID& ID, const std::string& name, const glm::mat3& matrix) = nullptr;
//void (*set_shader_mat4_float_value)(const ObjID& ID, const std::string& name, const glm::mat4& matrix) = nullptr;
//void (*set_shader_vec2_float_value)(const ObjID& ID, const std::string& name, const glm::vec2& matrix) = nullptr;
//void (*set_shader_vec3_float_value)(const ObjID& ID, const std::string& name, const glm::vec3& matrix) = nullptr;
//void (*set_shader_vec4_float_value)(const ObjID& ID, const std::string& name, const glm::vec4& matrix) = nullptr;

//void (*set_depth_mask)(bool mask) = nullptr;
//void (*set_stencil_mask)(byte mask) = nullptr;
//void (*set_stencil_func)(Engine::CompareFunc func, int ref, byte mask) = nullptr;
//void (*set_stencil_option)(Engine::StencilOption stencil_fail, Engine::StencilOption depth_fail,
//                           Engine::StencilOption pass) = nullptr;

//void (*create_ssbo)(ObjID& ID) = nullptr;
//void (*bind_ssbo)(const ObjID& ID) = nullptr;
//void (*set_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, BufferUsage mode) = nullptr;
//void (*update_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, std::size_t offset) = nullptr;
