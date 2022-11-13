#include <api_funcs.hpp>
#include <vector>


namespace Engine
{
    // Api initialization
    bool (*api_init)(std::vector<int> params) = nullptr;
    void (*api_terminate)() = nullptr;
    void (*set_logger)(Logger*& logger) = nullptr;
    void* (*api_init_window)(class SDL_Window* window) = nullptr;
    void (*api_destroy_window)() = nullptr;

    //////////////////////// TEXTURE 2D ////////////////////////
    void (*create_texture)(ObjID& ID, const TextureParams& params) = nullptr;
    void (*destroy_object)(ObjID& ID) = nullptr;
    void (*bind_texture)(const ObjID& ID, unsigned int num) = nullptr;
    const TextureParams* (*get_param_texture)(const ObjID& ID) = nullptr;
    void (*link_object)(const ObjID& src_ID, ObjID& dest_ID) = nullptr;
    int (*get_base_level_texture)(const ObjID& ID) = nullptr;
    void (*set_base_level_texture)(const ObjID& ID, int level) = nullptr;
    void (*set_depth_stencil_mode_texture)(const ObjID& ID, DepthStencilMode mode) = nullptr;
    DepthStencilMode (*get_depth_stencil_mode_texture)(const ObjID& ID) = nullptr;
    CompareFunc (*get_compare_func_texture)(const ObjID& ID) = nullptr;
    void (*set_compare_func_texture)(const ObjID& ID, CompareFunc func) = nullptr;
    CompareMode (*get_compare_mode_texture)(const ObjID& ID) = nullptr;
    void (*set_compare_mode_texture)(const ObjID& ID, CompareMode mode) = nullptr;
    TextureFilter (*get_min_filter_texture)(const ObjID& ID) = nullptr;
    TextureFilter (*get_mag_filter_texture)(const ObjID& ID) = nullptr;
    void (*set_min_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
    void (*set_mag_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
    void (*set_min_lod_level_texture)(const ObjID& ID, int level) = nullptr;
    void (*set_max_lod_level_texture)(const ObjID& ID, int level) = nullptr;
    void (*set_max_mipmap_level_texture)(const ObjID& ID, int level) = nullptr;
    int (*get_min_lod_level_texture)(const ObjID& ID) = nullptr;
    int (*get_max_lod_level_texture)(const ObjID& ID) = nullptr;
    int (*get_max_mipmap_level_texture)(const ObjID& ID) = nullptr;
    void (*set_swizzle_texture)(const ObjID& ID, const SwizzleRGBA& value) = nullptr;
    SwizzleRGBA (*get_swizzle_texture)(const ObjID& ID) = nullptr;
    void (*set_wrap_s_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    void (*set_wrap_t_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    void (*set_wrap_r_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    WrapValue (*get_wrap_s_texture)(const ObjID& ID) = nullptr;
    WrapValue (*get_wrap_t_texture)(const ObjID& ID) = nullptr;
    WrapValue (*get_wrap_r_texture)(const ObjID& ID) = nullptr;
    void (*copy_read_buffer_to_texture_2D)(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap) = nullptr;
    void (*texture_2D_update_from_current_read_buffer)(const ObjID& ID, const Size2D& size, const Offset2D& offset,
                                                       const Point2D& pos, int mipmap) = nullptr;
    void (*gen_texture_2D)(const ObjID& ID, const Size2D& size, int level, void* data) = nullptr;
    void (*update_texture_2D)(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data) = nullptr;
    void (*get_size_texture)(const ObjID& ID, Size3D& size, int level) = nullptr;
    void (*generate_texture_mipmap)(const ObjID& _ID) = nullptr;
    void (*read_texture_2D_data)(const ObjID& ID, std::vector<byte>& data, int level) = nullptr;
    ObjID (*texture_id)(const ObjID& ID) = nullptr;
    void (*cubemap_texture_attach_2d_texture)(const ObjID& ID, const ObjID& texture, TextureCubeMapFace index,
                                              int level) = nullptr;
    void (*cubemap_texture_attach_data)(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
                                        void* data) = nullptr;

    void (*generate_mesh)(ObjID& ID) = nullptr;
    void (*set_mesh_data)(const ObjID& ID, MeshInfo& info, void* data) = nullptr;
    void (*update_mesh_attributes)(const ObjID& ID, MeshInfo& info) = nullptr;
    void (*draw_mesh)(const ObjID& ID, Primitive primitive, std::size_t vertices, unsigned int start_index) = nullptr;
    void (*update_mesh_date)(const ObjID& ID, std::size_t offset, std::size_t size, void* data) = nullptr;

    void (*gen_framebuffer)(ObjID& ID, FrameBufferType type) = nullptr;
    void (*clear_frame_buffer)(const ObjID& _M_ID, BufferType type) = nullptr;
    void (*bind_framebuffer)(const ObjID& ID) = nullptr;
    void (*set_framebuffer_viewport)(const ObjID& ID, const Point2D& pos, const Size2D& size) = nullptr;
    void (*attach_texture_to_framebuffer)(const ObjID& ID, const ObjID& texture, FrameBufferAttach attach, unsigned int num,
                                          int level) = nullptr;

    void (*api_enable)(Engine::EnableCap cap) = nullptr;
    void (*api_disable)(Engine::EnableCap cap) = nullptr;
    void (*set_blend_func)(Engine::BlendFunc, Engine::BlendFunc) = nullptr;
    void (*set_depth_func)(Engine::CompareFunc func) = nullptr;
    float (*get_current_line_rendering_width)() = nullptr;
    void (*set_line_rendering_width)(float value) = nullptr;

    // Shader system
    void (*create_shader)(ObjID& ID, const ShaderParams& params) = nullptr;
    void (*use_shader)(const ObjID& ID) = nullptr;
    void (*set_shader_float_value)(const ObjID& ID, const std::string& name, float value) = nullptr;
    void (*set_shader_int_value)(const ObjID& ID, const std::string& name, int value) = nullptr;
    void (*set_shader_mat3_float_value)(const ObjID& ID, const std::string& name, const glm::mat3& matrix) = nullptr;
    void (*set_shader_mat4_float_value)(const ObjID& ID, const std::string& name, const glm::mat4& matrix) = nullptr;
    void (*set_shader_vec2_float_value)(const ObjID& ID, const std::string& name, const glm::vec2& matrix) = nullptr;
    void (*set_shader_vec3_float_value)(const ObjID& ID, const std::string& name, const glm::vec3& matrix) = nullptr;
    void (*set_shader_vec4_float_value)(const ObjID& ID, const std::string& name, const glm::vec4& matrix) = nullptr;


    std::vector<std::pair<void**, const char*>> _M_extern_funcs = {
            {(void**) &set_logger, "api_set_logger"},
            {(void**) &api_init, "api_init"},
            {(void**) &api_terminate, "api_terminate"},
            {(void**) &api_init_window, "api_init_window"},
            {(void**) &api_destroy_window, "api_destroy_window"},
            {(void**) &create_texture, "api_create_texture_instance"},
            {(void**) &destroy_object, "api_destroy_object_instance"},
            {(void**) &bind_texture, "api_bind_texture"},
            {(void**) &get_param_texture, "api_param_texture"},
            {(void**) &link_object, "api_link_object"},
            {(void**) &get_base_level_texture, "api_get_base_level_texture"},
            {(void**) &set_base_level_texture, "api_set_base_level_texture"},
            {(void**) &set_depth_stencil_mode_texture, "api_set_depth_stencil_texture"},
            {(void**) &get_depth_stencil_mode_texture, "api_get_depth_stencil_texture"},
            {(void**) &get_compare_func_texture, "api_get_compare_func_texture"},
            {(void**) &set_compare_func_texture, "api_set_compare_func_texture"},
            {(void**) &get_compare_mode_texture, "api_get_compare_mode_texture"},
            {(void**) &set_compare_mode_texture, "api_set_compare_mode_texture"},
            {(void**) &get_min_filter_texture, "api_get_min_filter_texture"},
            {(void**) &get_mag_filter_texture, "api_get_mag_filter_texture"},
            {(void**) &set_min_filter_texture, "api_set_min_filter_texture"},
            {(void**) &set_mag_filter_texture, "api_set_mag_filter_texture"},
            {(void**) &set_min_lod_level_texture, "api_set_min_lod_level_texture"},
            {(void**) &set_max_lod_level_texture, "api_set_max_lod_level_texture"},
            {(void**) &set_max_mipmap_level_texture, "api_set_max_mipmap_level_texture"},
            {(void**) &get_min_lod_level_texture, "api_get_min_lod_level_texture"},
            {(void**) &get_max_lod_level_texture, "api_get_max_lod_level_texture"},
            {(void**) &get_max_mipmap_level_texture, "api_get_max_mipmap_level_texture"},
            {(void**) &set_swizzle_texture, "api_set_swizzle_texture"},
            {(void**) &get_swizzle_texture, "api_get_swizzle_texture"},
            {(void**) &set_wrap_s_texture, "api_set_wrap_s_texture"},
            {(void**) &set_wrap_t_texture, "api_set_wrap_t_texture"},
            {(void**) &set_wrap_r_texture, "api_set_wrap_r_texture"},
            {(void**) &get_wrap_s_texture, "api_get_wrap_s_texture"},
            {(void**) &get_wrap_t_texture, "api_get_wrap_t_texture"},
            {(void**) &get_wrap_r_texture, "api_get_wrap_r_texture"},
            {(void**) &copy_read_buffer_to_texture_2D, "api_copy_read_buffer_to_texture_2D"},
            {(void**) &gen_texture_2D, "api_gen_texture_2D"},
            {(void**) &generate_texture_mipmap, "api_generate_texture_mipmap"},
            {(void**) &get_size_texture, "api_get_size_texture"},
            {(void**) &update_texture_2D, "api_update_texture_2D"},
            {(void**) &texture_2D_update_from_current_read_buffer, "api_texture_2D_update_from_current_read_buffer"},
            {(void**) &read_texture_2D_data, "api_read_texture_2D_data"},
            {(void**) &texture_id, "api_texture_id"},
            {(void**) &generate_mesh, "api_generate_mesh"},
            {(void**) &set_mesh_data, "api_set_mesh_data"},
            {(void**) &update_mesh_attributes, "api_update_mesh_attributes"},
            {(void**) &draw_mesh, "api_draw_mesh"},
            {(void**) &update_mesh_date, "api_update_mesh_data"},
            {(void**) &gen_framebuffer, "api_gen_framebuffer"},
            {(void**) &clear_frame_buffer, "api_clear_frame_buffer"},
            {(void**) &bind_framebuffer, "api_bind_framebuffer"},
            {(void**) &set_framebuffer_viewport, "api_set_framebuffer_viewport"},
            {(void**) &attach_texture_to_framebuffer, "api_attach_texture_to_framebuffer"},
            {(void**) &cubemap_texture_attach_2d_texture, "api_cubemap_texture_attach_2d_texture"},
            {(void**) &cubemap_texture_attach_data, "api_cubemap_texture_attach_data"},
            {(void**) &api_enable, "api_enable"},
            {(void**) &api_disable, "api_disable"},
            {(void**) &set_blend_func, "api_blend_func"},
            {(void**) &set_depth_func, "api_set_depth_func"},
            {(void**) &get_current_line_rendering_width, "api_get_current_line_rendering_width"},
            {(void**) &set_line_rendering_width, "api_set_line_rendering_width"},
            {(void**) &create_shader, "api_create_shader"},
            {(void**) &use_shader, "api_use_shader"},
            {(void**) &set_shader_float_value, "api_set_shader_float_value"},
            {(void**) &set_shader_int_value, "api_set_shader_int_value"},
            {(void**) &set_shader_mat3_float_value, "api_set_shader_mat3_float_value"},
            {(void**) &set_shader_mat4_float_value, "api_set_shader_mat4_float_value"},
            {(void**) &set_shader_vec2_float_value, "api_set_shader_vec2_float_value"},
            {(void**) &set_shader_vec3_float_value, "api_set_shader_vec3_float_value"},
            {(void**) &set_shader_vec4_float_value, "api_set_shader_vec4_float_value"},

    };// namespace Engine
}// namespace Engine
