#pragma once
#include <Core/logger.hpp>
#include <api.hpp>
#include <opengl_export.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_Object {
    public:
        GLuint _M_instance_id;

        virtual void* instance_address();

        template<typename Type>
        Type* get_instance_by_type()
        {
            return reinterpret_cast<Type*>(instance_address());
        }

        virtual ~OpenGL_Object();
    };


    OPENGL_EXPORT struct OpenGL : public GraphicApiInterface::ApiInterface {
        Logger* _M_current_logger = nullptr;
        void* _M_context = nullptr;
        float _M_current_line_rendering_width = 1.f;
        static OpenGL* _M_api;

    public:
        static OpenGL_Object* instance(const ObjID& ID);
        static OpenGL* instance();
        static ObjID get_object_id(OpenGL_Object* object);
        int_t get_current_binding(GLint type);
        GLuint get_gl_type_of_texture(const ObjID& ID);

    public:
        OpenGL();
        OpenGL& logger(Logger*) override;
        void* init_window(SDL_Window*) override;
        OpenGL& destroy_window() override;
        OpenGL& destroy_object(ObjID&) override;

        ///////////////// TEXTURE PART /////////////////
        OpenGL& create_texture(ObjID&, const TextureParams&) override;
        OpenGL& bind_texture(const ObjID&, TextureAttachIndex) override;
        const TextureParams* param_texture(const ObjID&) override;
        int_t base_level_texture(const ObjID&) override;
        OpenGL& base_level_texture(const ObjID&, int_t) override;
        OpenGL& depth_stencil_mode_texture(const ObjID&, DepthStencilMode) override;
        DepthStencilMode depth_stencil_mode_texture(const ObjID&) override;
        CompareFunc compare_func_texture(const ObjID&) override;
        OpenGL& compare_func_texture(const ObjID&, CompareFunc) override;
        CompareMode compare_mode_texture(const ObjID&) override;
        OpenGL& compare_mode_texture(const ObjID&, CompareMode) override;
        TextureFilter min_filter_texture(const ObjID&) override;
        TextureFilter mag_filter_texture(const ObjID&) override;
        OpenGL& min_filter_texture(const ObjID&, TextureFilter) override;
        OpenGL& mag_filter_texture(const ObjID&, TextureFilter) override;
        OpenGL& min_lod_level_texture(const ObjID&, int_t) override;
        OpenGL& max_lod_level_texture(const ObjID&, int_t) override;
        OpenGL& max_mipmap_level_texture(const ObjID&, int_t) override;
        int_t min_lod_level_texture(const ObjID&) override;
        int_t max_lod_level_texture(const ObjID&) override;
        int_t max_mipmap_level_texture(const ObjID&) override;
        OpenGL& swizzle_texture(const ObjID&, const SwizzleRGBA&) override;
        SwizzleRGBA swizzle_texture(const ObjID&) override;
        OpenGL& wrap_s_texture(const ObjID&, const WrapValue&) override;
        OpenGL& wrap_t_texture(const ObjID&, const WrapValue&) override;
        OpenGL& wrap_r_texture(const ObjID&, const WrapValue&) override;
        WrapValue wrap_s_texture(const ObjID&) override;
        WrapValue wrap_t_texture(const ObjID&) override;
        WrapValue wrap_r_texture(const ObjID&) override;
        OpenGL& copy_read_buffer_to_texture_2D(const ObjID&, const Size2D&, const Point2D&, int_t) override;
        OpenGL& texture_2D_update_from_current_read_buffer(const ObjID&, const Size2D&, const Offset2D&, const Point2D&,
                                                           int_t) override;
        OpenGL& gen_texture_2D(const ObjID&, const Size2D&, int_t, void*) override;
        OpenGL& update_texture_2D(const ObjID&, const Size2D&, const Offset2D&, int_t, void*) override;
        OpenGL& texture_size(const ObjID&, Size3D&, int_t) override;
        OpenGL& generate_texture_mipmap(const ObjID&) override;
        OpenGL& read_texture_2D_data(const ObjID&, std::vector<byte>& data, int_t) override;
        ObjID texture_id(const ObjID&) override;
        OpenGL& cubemap_texture_attach_2d_texture(const ObjID&, const ObjID&, TextureCubeMapFace, int) override;
        OpenGL& cubemap_texture_attach_data(const ObjID&, TextureCubeMapFace, const Size2D&, int, void*) override;


        ////////////////////  MESH PART ////////////////////
        OpenGL& generate_mesh(ObjID&) override;
        OpenGL& mesh_data(const ObjID&, std::size_t, DrawMode, void*) override;
        OpenGL& update_mesh_attributes(const ObjID&, const MeshInfo&) override;
        OpenGL& draw_mesh(const ObjID&, Primitive, std::size_t, std::size_t) override;
        OpenGL& update_mesh_data(const ObjID&, std::size_t, std::size_t, void*) override;
        OpenGL& mesh_indexes_array(const ObjID&, const MeshInfo&, std::size_t, const BufferValueType&, void*) override;
        OpenGL& gen_framebuffer(ObjID&, FrameBufferType) override;
        OpenGL& clear_frame_buffer(const ObjID&, BufferType) override;
        OpenGL& bind_framebuffer(const ObjID&) override;
        OpenGL& framebuffer_viewport(const Point2D&, const Size2D&) override;
        OpenGL& attach_texture_to_framebuffer(const ObjID&, const ObjID&, FrameBufferAttach, TextureAttachIndex,
                                              int_t) override;
        OpenGL& enable(Engine::EnableCap) override;
        OpenGL& disable(Engine::EnableCap) override;
        OpenGL& blend_func(Engine::BlendFunc, Engine::BlendFunc) override;
        OpenGL& depth_func(Engine::DepthFunc) override;
        float line_rendering_width() override;
        OpenGL& line_rendering_width(float) override;
        OpenGL& create_shader(ObjID&, const ShaderParams&) override;
        OpenGL& use_shader(const ObjID&) override;
        OpenGL& shader_value(const ObjID&, const std::string&, float) override;
        OpenGL& shader_value(const ObjID&, const std::string&, int_t) override;
        OpenGL& shader_value(const ObjID&, const std::string&, const glm::mat3&) override;
        OpenGL& shader_value(const ObjID&, const std::string&, const glm::mat4&) override;
        OpenGL& shader_value(const ObjID&, const std::string&, const glm::vec2&) override;
        OpenGL& shader_value(const ObjID&, const std::string&, const glm::vec3&) override;
        OpenGL& shader_value(const ObjID&, const std::string&, const glm::vec4&) override;
        OpenGL& depth_mask(bool) override;
        OpenGL& stencil_mask(byte) override;
        OpenGL& stencil_func(Engine::CompareFunc, int_t, byte) override;
        OpenGL& stencil_option(Engine::StencilOption, Engine::StencilOption, Engine::StencilOption) override;
        OpenGL& create_ssbo(ObjID&) override;
        OpenGL& bind_ssbo(const ObjID&, int_t slot) override;
        OpenGL& ssbo_data(const ObjID&, void*, std::size_t, BufferUsage) override;
        OpenGL& update_ssbo_data(const ObjID&, void*, std::size_t, std::size_t) override;
        OpenGL& swap_buffer(SDL_Window* window) override;
        OpenGL& swap_interval(int_t value) override;
        OpenGL& clear_color(const Color& color) override;
        OpenGL& on_window_size_changed() override;

        ~OpenGL();
    };

#define check(ID, _return)                                                                                             \
    auto obj = instance(ID);                                                                                           \
    if (!obj)                                                                                                          \
        return _return;
}// namespace Engine
