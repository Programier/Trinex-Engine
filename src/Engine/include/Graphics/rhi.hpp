#pragma once
#include <Core/structures.hpp>

struct ImGuiContext;
struct ImDrawData;

namespace Engine
{
    class RenderViewport;
    struct WindowConfig;
    struct SamplerCreateInfo;

    class VertexShader;
    class FragmentShader;
    class TessellationControlShader;
    class TessellationShader;
    class GeometryShader;
    class Shader;
    class Pipeline;
    class Sampler;
    class RenderSurface;
    class Texture2D;
    struct GlobalShaderParameters;


    struct RHI_Object {
        FORCE_INLINE virtual Identifier internal_type()// For internal usage
        {
            return 0;
        }

        FORCE_INLINE virtual bool is_destroyable() const
        {
            return true;
        }

        virtual ~RHI_Object() = default;
    };

    struct RHI_BindingObject : RHI_Object {
        virtual void bind(BindLocation location) = 0;
    };

    struct RHI_Sampler : RHI_BindingObject {
    };

    struct RHI_Texture : RHI_BindingObject {
        virtual void bind_combined(RHI_Sampler* sampler, BindLocation location) = 0;

        // Valid only for RenderSurface!
        virtual void clear_color(const Color& color)                = 0;
        virtual void clear_depth_stencil(float depth, byte stencil) = 0;
    };

    struct RHI_Shader : RHI_Object {
    };

    struct RHI_Pipeline : RHI_Object {
        virtual void bind() = 0;
    };

    struct RHI_Buffer : RHI_Object {
        virtual void update(size_t offset, size_t size, const byte* data) = 0;
    };

    struct RHI_VertexBuffer : RHI_Buffer {
        virtual void bind(byte stream_index, size_t stride, size_t offset) = 0;
    };

    struct RHI_IndexBuffer : RHI_Buffer {
        virtual void bind(size_t offset) = 0;
    };

    struct RHI_SSBO : RHI_Buffer {
        virtual void bind(BindLocation location) = 0;
    };

    struct RHI_Viewport : RHI_Object {

        virtual void begin_render() = 0;
        virtual void end_render()   = 0;

        virtual void vsync(bool flag)                  = 0;
        virtual void on_resize(const Size2D& new_size) = 0;
        virtual void bind()                            = 0;
        virtual void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
                                 SamplerFilter filter) = 0;
        virtual void clear_color(const Color& color)   = 0;
    };

    struct ENGINE_EXPORT RHI {
        struct Info {
            String name;
            String renderer;
            class Struct* struct_instance = nullptr;
        } info;

        virtual RHI& initialize(class Window* window)   = 0;
        virtual void* context()                         = 0;
        virtual RHI& destroy_object(RHI_Object* object) = 0;

        virtual RHI& imgui_init(ImGuiContext*)                = 0;
        virtual RHI& imgui_terminate(ImGuiContext*)           = 0;
        virtual RHI& imgui_new_frame(ImGuiContext*)           = 0;
        virtual RHI& imgui_render(ImGuiContext*, ImDrawData*) = 0;


        virtual RHI& draw(size_t vertex_count, size_t vertices_offset)                                 = 0;
        virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset) = 0;
        virtual RHI& draw_instanced(size_t vertex_count, size_t vertex_offset, size_t instances)       = 0;
        virtual RHI& draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset,
                                            size_t instances)                                          = 0;

        virtual RHI& begin_render() = 0;
        virtual RHI& end_render()   = 0;
        virtual RHI& wait_idle()    = 0;

        virtual void bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil) = 0;
        virtual void viewport(const ViewPort& viewport)                                                              = 0;
        virtual ViewPort viewport()                                                                                  = 0;

        virtual RHI_Sampler* create_sampler(const Sampler*)                                               = 0;
        virtual RHI_Texture* create_texture_2d(const Texture2D*)                                          = 0;
        virtual RHI_Shader* create_vertex_shader(const VertexShader* shader)                              = 0;
        virtual RHI_Shader* create_tesselation_control_shader(const TessellationControlShader* shader)    = 0;
        virtual RHI_Shader* create_tesselation_shader(const TessellationShader* shader)                   = 0;
        virtual RHI_Shader* create_geometry_shader(const GeometryShader* shader)                          = 0;
        virtual RHI_Shader* create_fragment_shader(const FragmentShader* shader)                          = 0;
        virtual RHI_Pipeline* create_pipeline(const Pipeline* pipeline)                                   = 0;
        virtual RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data, RHIBufferType type) = 0;
        virtual RHI_IndexBuffer* create_index_buffer(size_t, const byte* data)                            = 0;
        virtual RHI_SSBO* create_ssbo(size_t size, const byte* data)                                      = 0;
        virtual RHI_Viewport* create_viewport(RenderViewport* viewport)                                   = 0;

        virtual RHI& push_global_params(const GlobalShaderParameters& params)             = 0;
        virtual RHI& pop_global_params()                                                  = 0;
        virtual RHI& update_local_parameter(const void* data, size_t size, size_t offset) = 0;

        virtual void push_debug_stage(const char* stage, const Color& color = {}) = 0;
        virtual void pop_debug_stage()                                            = 0;

        virtual ~RHI(){};
    };

    ENGINE_EXPORT extern RHI* rhi;
}// namespace Engine
