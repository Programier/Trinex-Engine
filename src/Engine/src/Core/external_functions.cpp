#include <api_funcs.hpp>
#include <vector>


namespace Engine
{
    // Api initialization
    ApiFunction<bool, std::vector<int>> api_init(L"api_init");
    ApiFunction<void> api_terminate(L"api_terminate");
    ApiFunction<void, Logger*&> set_logger(L"set_logger");
    ApiFunction<void*, class SDL_Window*> api_init_window(L"api_init_window");
    ApiFunction<void> api_destroy_window(L"api_destroy_window");
    ApiFunction<void, ObjID&, const TextureParams&> create_texture(L"create_texture");
    ApiFunction<void, ObjID&> destroy_object(L"destroy_object");
    ApiFunction<void, const ObjID&, unsigned int> bind_texture(L"bind_texture");
    ApiFunction<const TextureParams*, const ObjID&> get_param_texture(L"get_param_texture");
    ApiFunction<void, const ObjID&, ObjID&> link_object(L"link_object");
    ApiFunction<int, const ObjID&> get_base_level_texture(L"get_base_level_texture");
    ApiFunction<void, const ObjID&, int> set_base_level_texture(L"set_base_level_texture");
    ApiFunction<void, const ObjID&, DepthStencilMode> set_depth_stencil_mode_texture(L"set_depth_stencil_mode_texture");
    ApiFunction<DepthStencilMode, const ObjID&> get_depth_stencil_mode_texture(L"get_depth_stencil_mode_texture");
    ApiFunction<CompareFunc, const ObjID&> get_compare_func_texture(L"get_compare_func_texture");
    ApiFunction<void, const ObjID&, CompareFunc> set_compare_func_texture(L"set_compare_func_texture");
    ApiFunction<CompareMode, const ObjID&> get_compare_mode_texture(L"get_compare_mode_texture");
    ApiFunction<void, const ObjID&, CompareMode> set_compare_mode_texture(L"set_compare_mode_texture");
    ApiFunction<TextureFilter, const ObjID&> get_min_filter_texture(L"get_mag_filter_texture");
    ApiFunction<TextureFilter, const ObjID&> get_mag_filter_texture(L"get_min_filter_texture");
    ApiFunction<void, const ObjID&, TextureFilter> set_min_filter_texture(L"set_mag_filter_texture");
    ApiFunction<void, const ObjID&, TextureFilter> set_mag_filter_texture(L"set_min_filter_texture");
    ApiFunction<void, const ObjID&, int> set_min_lod_level_texture(L"set_min_lod_level_texture");
    ApiFunction<void, const ObjID&, int> set_max_lod_level_texture(L"set_max_lod_level_texture");
    ApiFunction<void, const ObjID&, int> set_max_mipmap_level_texture(L"set_max_mipmap_level_texture");
    ApiFunction<int, const ObjID&> get_min_lod_level_texture(L"get_min_lod_level_texture");
    ApiFunction<int, const ObjID&> get_max_lod_level_texture(L"get_max_lod_level_texture");
    ApiFunction<int, const ObjID&> get_max_mipmap_level_texture(L"get_max_mipmap_level_texture");
    ApiFunction<void, const ObjID&, const SwizzleRGBA&> set_swizzle_texture(L"set_swizzle_texture");
    ApiFunction<SwizzleRGBA, const ObjID&> get_swizzle_texture(L"get_swizzle_texture");
    ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_s_texture(L"set_wrap_s_texture");
    ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_t_texture(L"set_wrap_t_texture");
    ApiFunction<void, const ObjID&, const WrapValue&> set_wrap_r_texture(L"set_wrap_r_texture");
    ApiFunction<WrapValue, const ObjID&> get_wrap_s_texture(L"get_wrap_s_texture");
    ApiFunction<WrapValue, const ObjID&> get_wrap_t_texture(L"get_wrap_t_texture");
    ApiFunction<WrapValue, const ObjID&> get_wrap_r_texture(L"get_wrap_r_texture");
    ApiFunction<void, const ObjID&, const Size2D&, const Point2D&, int> copy_read_buffer_to_texture_2D(L"copy_read_buffer_to_texture_2D");
    ApiFunction<void, const ObjID&, const Size2D&, const Offset2D&, const Point2D&, int>
            texture_2D_update_from_current_read_buffer(L"texture_2D_update_from_current_read_buffer");
    ApiFunction<void, const ObjID&, const Size2D&, int, void*> gen_texture_2D(L"gen_texture_2D");
    ApiFunction<void, const ObjID&, const Size2D&, const Offset2D&, int, void*> update_texture_2D(L"update_texture_2D");
    ApiFunction<void, const ObjID&, Size3D&, int> get_size_texture(L"generate_texture_mipmap");
    ApiFunction<void, const ObjID&> generate_texture_mipmap(L"get_size_texture");
    ApiFunction<void, const ObjID&, std::vector<byte>&, int> read_texture_2D_data(L"read_texture_2D_data");
    ApiFunction<ObjID, const ObjID&> texture_id(L"texture_id");
    ApiFunction<void, const ObjID&, const ObjID&, TextureCubeMapFace, int> cubemap_texture_attach_2d_texture(L"cubemap_texture_attach_2d_texture");
    ApiFunction<void, const ObjID&, TextureCubeMapFace, const Size2D&, int, void*> cubemap_texture_attach_data(L"cubemap_texture_attach_data");
    ApiFunction<void, ObjID&> generate_mesh(L"generate_mesh");
    ApiFunction<void, const ObjID&, std::size_t, DrawMode, void*> set_mesh_data(L"set_mesh_data");
    ApiFunction<void, const ObjID&, const MeshInfo&> update_mesh_attributes(L"update_mesh_attributes");
    ApiFunction<void, const ObjID&, Primitive, std::size_t, std::size_t> draw_mesh(L"draw_mesh");
    ApiFunction<void, const ObjID&, std::size_t, std::size_t, void*> update_mesh_data(L"update_mesh_data");
    ApiFunction<void, const ObjID&, const MeshInfo&, std::size_t, const BufferValueType&, void*> set_mesh_indexes_array(L"set_mesh_indexes_array");
    ApiFunction<void, ObjID&, FrameBufferType> gen_framebuffer(L"gen_framebuffer");
    ApiFunction<void, const ObjID&, BufferType> clear_frame_buffer(L"clear_frame_buffer");
    ApiFunction<void, const ObjID&> bind_framebuffer(L"bind_framebuffer");
    ApiFunction<void, const ObjID&, const Point2D&, const Size2D&> set_framebuffer_viewport(L"set_framebuffer_viewport");
    ApiFunction<void, const ObjID&, const ObjID&, FrameBufferAttach, unsigned int, int> attach_texture_to_framebuffer(L"attach_texture_to_framebuffer");
    ApiFunction<void, Engine::EnableCap> api_enable(L"api_enable");
    ApiFunction<void, Engine::EnableCap> api_disable(L"api_disable");
    ApiFunction<void, Engine::BlendFunc, Engine::BlendFunc> set_blend_func(L"set_blend_func");
    ApiFunction<void, Engine::CompareFunc> set_depth_func(L"set_depth_func");
    ApiFunction<float> get_current_line_rendering_width(L"get_current_line_rendering_width");
    ApiFunction<void, float> set_line_rendering_width(L"set_line_rendering_width");
    ApiFunction<void, ObjID&, const ShaderParams&> create_shader(L"create_shader");
    ApiFunction<void, const ObjID&> use_shader(L"use_shader");
    ApiFunction<void, const ObjID&, const std::string&, float> set_shader_float_value(L"set_shader_float_value");
    ApiFunction<void, const ObjID&, const std::string&, int> set_shader_int_value(L"set_shader_int_value");
    ApiFunction<void, const ObjID&, const std::string&, const glm::mat3&> set_shader_mat3_float_value(L"set_shader_mat3_float_value");
    ApiFunction<void, const ObjID&, const std::string&, const glm::mat4&> set_shader_mat4_float_value(L"set_shader_mat4_float_value");
    ApiFunction<void, const ObjID&, const std::string&, const glm::vec2&> set_shader_vec2_float_value(L"set_shader_vec2_float_value");
    ApiFunction<void, const ObjID&, const std::string&, const glm::vec3&> set_shader_vec3_float_value(L"set_shader_vec3_float_value");
    ApiFunction<void, const ObjID&, const std::string&, const glm::vec4&> set_shader_vec4_float_value(L"set_shader_vec4_float_value");
    ApiFunction<void, bool> set_depth_mask(L"set_depth_mask");
    ApiFunction<void, byte> set_stencil_mask(L"set_stencil_mask");
    ApiFunction<void, Engine::CompareFunc, int, byte> set_stencil_func(L"set_stencil_func");
    ApiFunction<void, Engine::StencilOption, Engine::StencilOption, Engine::StencilOption> set_stencil_option(L"set_stencil_option");
    ApiFunction<void, ObjID&> create_ssbo(L"create_ssbo");
    ApiFunction<void, const ObjID&> bind_ssbo(L"bind_ssbo");
    ApiFunction<void, const ObjID&, void*, std::size_t, BufferUsage> set_ssbo_data(L"set_ssbo_data");
    ApiFunction<void, const ObjID&, void*, std::size_t, std::size_t> update_ssbo_data(L"update_ssbo_data");


    //    bool (*api_init)(std::vector<int> params) = nullptr;
    //    void (*api_terminate)() = nullptr;
    //    void (*set_logger)(Logger*& logger) = nullptr;
    //    void* (*api_init_window)(class SDL_Window* window) = nullptr;
    //    void (*api_destroy_window)() = nullptr;

    //    //////////////////////// TEXTURE 2D ////////////////////////
    //    void (*create_texture)(ObjID& ID, const TextureParams& params) = nullptr;
    //    void (*destroy_object)(ObjID& ID) = nullptr;
    //    void (*bind_texture)(const ObjID& ID, unsigned int num) = nullptr;
    //    const TextureParams* (*get_param_texture)(const ObjID& ID) = nullptr;
    //    void (*link_object)(const ObjID& src_ID, ObjID& dest_ID) = nullptr;
    //    int (*get_base_level_texture)(const ObjID& ID) = nullptr;
    //    void (*set_base_level_texture)(const ObjID& ID, int level) = nullptr;
    //    void (*set_depth_stencil_mode_texture)(const ObjID& ID, DepthStencilMode mode) = nullptr;
    //    DepthStencilMode (*get_depth_stencil_mode_texture)(const ObjID& ID) = nullptr;
    //    CompareFunc (*get_compare_func_texture)(const ObjID& ID) = nullptr;
    //    void (*set_compare_func_texture)(const ObjID& ID, CompareFunc func) = nullptr;
    //    CompareMode (*get_compare_mode_texture)(const ObjID& ID) = nullptr;
    //    void (*set_compare_mode_texture)(const ObjID& ID, CompareMode mode) = nullptr;
    //    TextureFilter (*get_min_filter_texture)(const ObjID& ID) = nullptr;
    //    TextureFilter (*get_mag_filter_texture)(const ObjID& ID) = nullptr;
    //    void (*set_min_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
    //    void (*set_mag_filter_texture)(const ObjID& ID, TextureFilter filter) = nullptr;
    //    void (*set_min_lod_level_texture)(const ObjID& ID, int level) = nullptr;
    //    void (*set_max_lod_level_texture)(const ObjID& ID, int level) = nullptr;
    //    void (*set_max_mipmap_level_texture)(const ObjID& ID, int level) = nullptr;
    //    int (*get_min_lod_level_texture)(const ObjID& ID) = nullptr;
    //    int (*get_max_lod_level_texture)(const ObjID& ID) = nullptr;
    //    int (*get_max_mipmap_level_texture)(const ObjID& ID) = nullptr;
    //    void (*set_swizzle_texture)(const ObjID& ID, const SwizzleRGBA& value) = nullptr;
    //    SwizzleRGBA (*get_swizzle_texture)(const ObjID& ID) = nullptr;
    //    void (*set_wrap_s_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    //    void (*set_wrap_t_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    //    void (*set_wrap_r_texture)(const ObjID& ID, const WrapValue& wrap) = nullptr;
    //    WrapValue (*get_wrap_s_texture)(const ObjID& ID) = nullptr;
    //    WrapValue (*get_wrap_t_texture)(const ObjID& ID) = nullptr;
    //    WrapValue (*get_wrap_r_texture)(const ObjID& ID) = nullptr;
    //    void (*copy_read_buffer_to_texture_2D)(const ObjID& ID, const Size2D& size, const Point2D& pos, int mipmap) = nullptr;
    //    void (*texture_2D_update_from_current_read_buffer)(const ObjID& ID, const Size2D& size, const Offset2D& offset,
    //                                                       const Point2D& pos, int mipmap) = nullptr;
    //    void (*gen_texture_2D)(const ObjID& ID, const Size2D& size, int level, void* data) = nullptr;
    //    void (*update_texture_2D)(const ObjID& ID, const Size2D& size, const Offset2D& offset, int level, void* data) = nullptr;
    //    void (*get_size_texture)(const ObjID& ID, Size3D& size, int level) = nullptr;
    //    void (*generate_texture_mipmap)(const ObjID& _ID) = nullptr;
    //    void (*read_texture_2D_data)(const ObjID& ID, std::vector<byte>& data, int level) = nullptr;
    //    ObjID (*texture_id)(const ObjID& ID) = nullptr;
    //    void (*cubemap_texture_attach_2d_texture)(const ObjID& ID, const ObjID& texture, TextureCubeMapFace index,
    //                                              int level) = nullptr;
    //    void (*cubemap_texture_attach_data)(const ObjID& ID, TextureCubeMapFace index, const Size2D& size, int level,
    //                                        void* data) = nullptr;

    //    void (*generate_mesh)(ObjID& ID) = nullptr;
    //    void (*set_mesh_data)(const ObjID& ID, std::size_t buffer_len, DrawMode mode, void* data) = nullptr;
    //    void (*update_mesh_attributes)(const ObjID& ID, const MeshInfo& info) = nullptr;
    //    void (*draw_mesh)(const ObjID& ID, Primitive primitive, std::size_t vertices, std::size_t offset) = nullptr;
    //    void (*update_mesh_data)(const ObjID& ID, std::size_t offset, std::size_t size, void* data) = nullptr;
    //    void (*set_mesh_indexes_array)(const ObjID& ID, const MeshInfo& info, std::size_t bytes,
    //                                   const BufferValueType& data_type, void*) = nullptr;

    //    void (*gen_framebuffer)(ObjID& ID, FrameBufferType type) = nullptr;
    //    void (*clear_frame_buffer)(const ObjID& _M_ID, BufferType type) = nullptr;
    //    void (*bind_framebuffer)(const ObjID& ID) = nullptr;
    //    void (*set_framebuffer_viewport)(const ObjID& ID, const Point2D& pos, const Size2D& size) = nullptr;
    //    void (*attach_texture_to_framebuffer)(const ObjID& ID, const ObjID& texture, FrameBufferAttach attach, unsigned int num,
    //                                          int level) = nullptr;

    //    void (*api_enable)(Engine::EnableCap cap) = nullptr;
    //    void (*api_disable)(Engine::EnableCap cap) = nullptr;
    //    void (*set_blend_func)(Engine::BlendFunc, Engine::BlendFunc) = nullptr;
    //    void (*set_depth_func)(Engine::CompareFunc func) = nullptr;
    //    float (*get_current_line_rendering_width)() = nullptr;
    //    void (*set_line_rendering_width)(float value) = nullptr;

    //    // Shader system
    //    void (*create_shader)(ObjID& ID, const ShaderParams& params) = nullptr;
    //    void (*use_shader)(const ObjID& ID) = nullptr;
    //    void (*set_shader_float_value)(const ObjID& ID, const std::string& name, float value) = nullptr;
    //    void (*set_shader_int_value)(const ObjID& ID, const std::string& name, int value) = nullptr;
    //    void (*set_shader_mat3_float_value)(const ObjID& ID, const std::string& name, const glm::mat3& matrix) = nullptr;
    //    void (*set_shader_mat4_float_value)(const ObjID& ID, const std::string& name, const glm::mat4& matrix) = nullptr;
    //    void (*set_shader_vec2_float_value)(const ObjID& ID, const std::string& name, const glm::vec2& matrix) = nullptr;
    //    void (*set_shader_vec3_float_value)(const ObjID& ID, const std::string& name, const glm::vec3& matrix) = nullptr;
    //    void (*set_shader_vec4_float_value)(const ObjID& ID, const std::string& name, const glm::vec4& matrix) = nullptr;

    //    void (*set_depth_mask)(bool mask) = nullptr;
    //    void (*set_stencil_mask)(byte mask) = nullptr;
    //    void (*set_stencil_func)(Engine::CompareFunc func, int ref, byte mask) = nullptr;
    //    void (*set_stencil_option)(Engine::StencilOption stencil_fail, Engine::StencilOption depth_fail,
    //                               Engine::StencilOption pass) = nullptr;

    //    void (*create_ssbo)(ObjID& ID) = nullptr;
    //    void (*bind_ssbo)(const ObjID& ID) = nullptr;
    //    void (*set_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, BufferUsage mode) = nullptr;
    //    void (*update_ssbo_data)(const ObjID& ID, void* data, std::size_t bytes, std::size_t offset) = nullptr;


    std::vector<std::pair<const IApiFunction*, const char*>> _M_extern_funcs = {
            {&dynamic_cast<IApiFunction&>(set_logger), "api_set_logger"},
            {&dynamic_cast<IApiFunction&>(api_init), "api_init"},
            {&dynamic_cast<IApiFunction&>(api_terminate), "api_terminate"},
            {&dynamic_cast<IApiFunction&>(api_init_window), "api_init_window"},
            {&dynamic_cast<IApiFunction&>(api_destroy_window), "api_destroy_window"},
            {&dynamic_cast<IApiFunction&>(create_texture), "api_create_texture_instance"},
            {&dynamic_cast<IApiFunction&>(destroy_object), "api_destroy_object_instance"},
            {&dynamic_cast<IApiFunction&>(bind_texture), "api_bind_texture"},
            {&dynamic_cast<IApiFunction&>(get_param_texture), "api_param_texture"},
            {&dynamic_cast<IApiFunction&>(link_object), "api_link_object"},
            {&dynamic_cast<IApiFunction&>(get_base_level_texture), "api_get_base_level_texture"},
            {&dynamic_cast<IApiFunction&>(set_base_level_texture), "api_set_base_level_texture"},
            {&dynamic_cast<IApiFunction&>(set_depth_stencil_mode_texture), "api_set_depth_stencil_texture"},
            {&dynamic_cast<IApiFunction&>(get_depth_stencil_mode_texture), "api_get_depth_stencil_texture"},
            {&dynamic_cast<IApiFunction&>(get_compare_func_texture), "api_get_compare_func_texture"},
            {&dynamic_cast<IApiFunction&>(set_compare_func_texture), "api_set_compare_func_texture"},
            {&dynamic_cast<IApiFunction&>(get_compare_mode_texture), "api_get_compare_mode_texture"},
            {&dynamic_cast<IApiFunction&>(set_compare_mode_texture), "api_set_compare_mode_texture"},
            {&dynamic_cast<IApiFunction&>(get_min_filter_texture), "api_get_min_filter_texture"},
            {&dynamic_cast<IApiFunction&>(get_mag_filter_texture), "api_get_mag_filter_texture"},
            {&dynamic_cast<IApiFunction&>(set_min_filter_texture), "api_set_min_filter_texture"},
            {&dynamic_cast<IApiFunction&>(set_mag_filter_texture), "api_set_mag_filter_texture"},
            {&dynamic_cast<IApiFunction&>(set_min_lod_level_texture), "api_set_min_lod_level_texture"},
            {&dynamic_cast<IApiFunction&>(set_max_lod_level_texture), "api_set_max_lod_level_texture"},
            {&dynamic_cast<IApiFunction&>(set_max_mipmap_level_texture), "api_set_max_mipmap_level_texture"},
            {&dynamic_cast<IApiFunction&>(get_min_lod_level_texture), "api_get_min_lod_level_texture"},
            {&dynamic_cast<IApiFunction&>(get_max_lod_level_texture), "api_get_max_lod_level_texture"},
            {&dynamic_cast<IApiFunction&>(get_max_mipmap_level_texture), "api_get_max_mipmap_level_texture"},
            {&dynamic_cast<IApiFunction&>(set_swizzle_texture), "api_set_swizzle_texture"},
            {&dynamic_cast<IApiFunction&>(get_swizzle_texture), "api_get_swizzle_texture"},
            {&dynamic_cast<IApiFunction&>(set_wrap_s_texture), "api_set_wrap_s_texture"},
            {&dynamic_cast<IApiFunction&>(set_wrap_t_texture), "api_set_wrap_t_texture"},
            {&dynamic_cast<IApiFunction&>(set_wrap_r_texture), "api_set_wrap_r_texture"},
            {&dynamic_cast<IApiFunction&>(get_wrap_s_texture), "api_get_wrap_s_texture"},
            {&dynamic_cast<IApiFunction&>(get_wrap_t_texture), "api_get_wrap_t_texture"},
            {&dynamic_cast<IApiFunction&>(get_wrap_r_texture), "api_get_wrap_r_texture"},
            {&dynamic_cast<IApiFunction&>(copy_read_buffer_to_texture_2D), "api_copy_read_buffer_to_texture_2D"},
            {&dynamic_cast<IApiFunction&>(gen_texture_2D), "api_gen_texture_2D"},
            {&dynamic_cast<IApiFunction&>(generate_texture_mipmap), "api_generate_texture_mipmap"},
            {&dynamic_cast<IApiFunction&>(get_size_texture), "api_get_size_texture"},
            {&dynamic_cast<IApiFunction&>(update_texture_2D), "api_update_texture_2D"},
            {&dynamic_cast<IApiFunction&>(texture_2D_update_from_current_read_buffer), "api_texture_2D_update_from_current_read_buffer"},
            {&dynamic_cast<IApiFunction&>(read_texture_2D_data), "api_read_texture_2D_data"},
            {&dynamic_cast<IApiFunction&>(texture_id), "api_texture_id"},
            {&dynamic_cast<IApiFunction&>(generate_mesh), "api_generate_mesh"},
            {&dynamic_cast<IApiFunction&>(set_mesh_indexes_array), "api_set_mesh_indexes_array"},
            {&dynamic_cast<IApiFunction&>(set_mesh_data), "api_set_mesh_data"},
            {&dynamic_cast<IApiFunction&>(update_mesh_attributes), "api_update_mesh_attributes"},
            {&dynamic_cast<IApiFunction&>(draw_mesh), "api_draw_mesh"},
            {&dynamic_cast<IApiFunction&>(update_mesh_data), "api_update_mesh_data"},
            {&dynamic_cast<IApiFunction&>(gen_framebuffer), "api_gen_framebuffer"},
            {&dynamic_cast<IApiFunction&>(clear_frame_buffer), "api_clear_frame_buffer"},
            {&dynamic_cast<IApiFunction&>(bind_framebuffer), "api_bind_framebuffer"},
            {&dynamic_cast<IApiFunction&>(set_framebuffer_viewport), "api_set_framebuffer_viewport"},
            {&dynamic_cast<IApiFunction&>(attach_texture_to_framebuffer), "api_attach_texture_to_framebuffer"},
            {&dynamic_cast<IApiFunction&>(cubemap_texture_attach_2d_texture), "api_cubemap_texture_attach_2d_texture"},
            {&dynamic_cast<IApiFunction&>(cubemap_texture_attach_data), "api_cubemap_texture_attach_data"},
            {&dynamic_cast<IApiFunction&>(api_enable), "api_enable"},
            {&dynamic_cast<IApiFunction&>(api_disable), "api_disable"},
            {&dynamic_cast<IApiFunction&>(set_blend_func), "api_blend_func"},
            {&dynamic_cast<IApiFunction&>(set_depth_func), "api_set_depth_func"},
            {&dynamic_cast<IApiFunction&>(get_current_line_rendering_width), "api_get_current_line_rendering_width"},
            {&dynamic_cast<IApiFunction&>(set_line_rendering_width), "api_set_line_rendering_width"},
            {&dynamic_cast<IApiFunction&>(create_shader), "api_create_shader"},
            {&dynamic_cast<IApiFunction&>(use_shader), "api_use_shader"},
            {&dynamic_cast<IApiFunction&>(set_shader_float_value), "api_set_shader_float_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_int_value), "api_set_shader_int_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_mat3_float_value), "api_set_shader_mat3_float_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_mat4_float_value), "api_set_shader_mat4_float_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_vec2_float_value), "api_set_shader_vec2_float_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_vec3_float_value), "api_set_shader_vec3_float_value"},
            {&dynamic_cast<IApiFunction&>(set_shader_vec4_float_value), "api_set_shader_vec4_float_value"},
            {&dynamic_cast<IApiFunction&>(set_depth_mask), "api_set_depth_mask"},
            {&dynamic_cast<IApiFunction&>(set_stencil_mask), "api_set_stencil_mask"},
            {&dynamic_cast<IApiFunction&>(set_stencil_func), "api_set_stencil_func"},
            {&dynamic_cast<IApiFunction&>(set_stencil_option), "api_set_stencil_option"},
            {&dynamic_cast<IApiFunction&>(create_ssbo), "api_create_ssbo"},
            {&dynamic_cast<IApiFunction&>(bind_ssbo), "api_bind_ssbo"},
            {&dynamic_cast<IApiFunction&>(set_ssbo_data), "api_set_ssbo_data"},
            {&dynamic_cast<IApiFunction&>(update_ssbo_data), "api_update_ssbo_data"},
    };
}// namespace Engine
