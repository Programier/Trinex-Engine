#pragma once


#include <Core/color_format.hpp>
#include <Core/colors.hpp>
#include <Core/rhi_initializers.hpp>

#define VIRTUAL_METHOD = 0

namespace Engine
{
    struct WindowInterface;
    struct WindowConfig;
    struct SamplerCreateInfo;

    class VertexShader;
    class FragmentShader;
    class Shader;
    class Pipeline;
    class Sampler;
    class RenderTarget;
    class RenderPass;
    class Texture;

    struct RHI_Object {
        FORCE_INLINE Identifier id() const
        {
            return reinterpret_cast<Identifier>(this);
        }

        virtual ~RHI_Object() = default;
    };

    struct RHI_BindingObject : RHI_Object {
        virtual void bind(BindLocation location) = 0;
    };

    struct RHI_Sampler : RHI_BindingObject {
    };

    struct RHI_Texture : RHI_BindingObject {
        virtual void generate_mipmap()                                          = 0;
        virtual void bind_combined(RHI_Sampler* sampler, BindLocation location) = 0;
        virtual void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                                       const byte* data)                        = 0;
    };

    struct RHI_RenderTarget : RHI_Object {
        virtual void bind()                                                   = 0;
        virtual void viewport(const ViewPort& viewport)                       = 0;
        virtual void scissor(const Scissor& scissor)                          = 0;
        virtual void clear_depth_stencil(const DepthStencilClearValue& value) = 0;
        virtual void clear_color(const ColorClearValue& color, byte layout)   = 0;
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
        virtual void bind(byte stream_index, size_t offset) = 0;
    };

    struct RHI_IndexBuffer : RHI_Buffer {
        virtual void bind(size_t offset) = 0;
    };

    struct RHI_UniformBuffer : RHI_Buffer {
        virtual void bind(BindLocation location) = 0;
    };

    struct RHI_SSBO : RHI_Buffer {
        virtual void bind(BindLocation location) = 0;
    };

    struct RHI_RenderPass : RHI_Object {
    };

    struct ENGINE_EXPORT RHI {
        virtual void* init_window(struct WindowInterface*, const WindowConfig& config) VIRTUAL_METHOD;
        virtual RHI& destroy_window() VIRTUAL_METHOD;
        virtual RHI& imgui_init() VIRTUAL_METHOD;
        virtual RHI& imgui_terminate() VIRTUAL_METHOD;
        virtual RHI& imgui_new_frame() VIRTUAL_METHOD;
        virtual RHI& imgui_render() VIRTUAL_METHOD;

        ///////////////// TEXTURE PART /////////////////
        virtual Identifier imgui_texture_id(const Identifier&) VIRTUAL_METHOD;

        virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset) VIRTUAL_METHOD;
        virtual RHI& draw(size_t vertex_count) VIRTUAL_METHOD;
        virtual RHI& swap_buffer() VIRTUAL_METHOD;
        virtual RHI& vsync(bool) VIRTUAL_METHOD;
        virtual bool vsync() VIRTUAL_METHOD;

        virtual RHI& on_window_size_changed() VIRTUAL_METHOD;
        virtual RHI& begin_render() VIRTUAL_METHOD;
        virtual RHI& end_render() VIRTUAL_METHOD;
        virtual RHI& wait_idle() VIRTUAL_METHOD;
        virtual String renderer() VIRTUAL_METHOD;

        virtual RHI_Sampler* create_sampler(const Sampler*)                                          = 0;
        virtual RHI_Texture* create_texture(const Texture*, const byte* data)                        = 0;
        virtual RHI_RenderTarget* window_render_target()                                             = 0;
        virtual RHI_RenderTarget* create_render_target(const RenderTarget* render_target)            = 0;
        virtual RHI_Shader* create_vertex_shader(const VertexShader* shader)                         = 0;
        virtual RHI_Shader* create_fragment_shader(const FragmentShader* shader)                     = 0;
        virtual RHI_Pipeline* create_pipeline(const Pipeline* pipeline)                              = 0;
        virtual RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data)                = 0;
        virtual RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) = 0;
        virtual RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data)              = 0;
        virtual RHI_SSBO* create_ssbo(size_t size, const byte* data)                                 = 0;
        virtual RHI_RenderPass* create_render_pass(const RenderPass* render_pass)                    = 0;
        virtual RHI_RenderPass* window_render_pass()                                                 = 0;
        virtual ColorFormatFeatures color_format_features(ColorFormat format)                        = 0;

        virtual void push_debug_stage(const char* stage, const Color& color = {}) = 0;
        virtual void pop_debug_stage()                                            = 0;

        virtual ~RHI(){};
    };

#undef VIRTUAL_METHOD
}// namespace Engine
