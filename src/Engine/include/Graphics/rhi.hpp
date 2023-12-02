#pragma once


#include <Core/color_format.hpp>
#include <Core/colors.hpp>
#include <Core/rhi_initializers.hpp>


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
        virtual void generate_mipmap()                                          = 0;
        virtual void bind_combined(RHI_Sampler* sampler, BindLocation location) = 0;
        virtual void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                                       const byte* data)                        = 0;
    };

    struct RHI_RenderTarget : RHI_Object {
        virtual Index bind()                                                  = 0;
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
        virtual void* init_window(struct WindowInterface*, const WindowConfig& config) = 0;
        virtual RHI& destroy_window()                                                  = 0;
        virtual RHI& imgui_init()                                                      = 0;
        virtual RHI& imgui_terminate()                                                 = 0;
        virtual RHI& imgui_new_frame()                                                 = 0;
        virtual RHI& imgui_render()                                                    = 0;


        virtual RHI& destroy_object(RHI_Object* object)        = 0;
        virtual Identifier imgui_texture_id(const Identifier&) = 0;

        virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset) = 0;
        virtual RHI& draw(size_t vertex_count)                                 = 0;
        virtual RHI& swap_buffer()                                             = 0;
        virtual RHI& vsync(bool)                                               = 0;
        virtual bool vsync()                                                   = 0;

        virtual RHI& on_window_size_changed() = 0;
        virtual RHI& begin_render()           = 0;
        virtual RHI& end_render()             = 0;
        virtual RHI& wait_idle()              = 0;
        virtual String renderer()             = 0;

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
        virtual size_t render_target_buffer_count()                                                  = 0;

        virtual void push_debug_stage(const char* stage, const Color& color = {}) = 0;
        virtual void pop_debug_stage()                                            = 0;

        virtual ~RHI(){};
    };
}// namespace Engine
