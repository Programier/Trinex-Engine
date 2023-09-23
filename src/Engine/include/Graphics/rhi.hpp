#pragma once


#include <Core/color.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/rhi_initializers.hpp>

#define VIRTUAL_METHOD = 0

namespace Engine
{
    struct WindowInterface;
    struct WindowConfig;
    struct SamplerCreateInfo;

    class VertexShader;
    class FragmentShader;
    class ShaderBase;
    class Pipeline;

    struct RHI_Object {
        FORCE_INLINE Identifier id() const
        {
            return reinterpret_cast<Identifier>(this);
        }

        virtual ~RHI_Object() = default;
    };

    struct RHI_BindingObject : RHI_Object {
        virtual void bind(BindingIndex binding, BindingIndex set) = 0;
    };

    struct RHI_Sampler : RHI_BindingObject {
    };

    struct RHI_Texture : RHI_BindingObject {
        virtual void generate_mipmap()                                                           = 0;
        virtual void bind_combined(RHI_Sampler* sampler, BindingIndex binding, BindingIndex set) = 0;
        virtual void update_texture_2D(const Size2D& size, const Offset2D& offset, MipMapLevel mipmap,
                                       const byte* data)                                         = 0;
    };

    struct RHI_FrameBuffer : RHI_Object {
        virtual void bind(uint_t buffer_index)                                = 0;
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
        virtual MappedMemory map_buffer()                                 = 0;
        virtual void unmap_buffer()                                       = 0;
        virtual void update(size_t offset, size_t size, const byte* data) = 0;
    };

    struct RHI_VertexBuffer : RHI_Buffer {
        virtual void bind(byte stream_index, size_t offset) = 0;
    };

    struct RHI_IndexBuffer : RHI_Buffer {
        virtual void bind(size_t offset) = 0;
    };

    struct RHI_UniformBuffer : RHI_Buffer {
        virtual void bind(BindingIndex binding, BindingIndex set) = 0;
    };

    struct RHI_SSBO : RHI_Buffer {
        virtual void bind(BindingIndex binding, BindingIndex set) = 0;
    };


    struct ENGINE_EXPORT RHI {
        virtual void* init_window(struct WindowInterface*, const WindowConfig& config) VIRTUAL_METHOD;
        virtual RHI& destroy_window() VIRTUAL_METHOD;
        virtual RHI& destroy_object(Identifier&) VIRTUAL_METHOD;
        virtual RHI& imgui_init() VIRTUAL_METHOD;
        virtual RHI& imgui_terminate() VIRTUAL_METHOD;
        virtual RHI& imgui_new_frame() VIRTUAL_METHOD;
        virtual RHI& imgui_render() VIRTUAL_METHOD;

        ///////////////// TEXTURE PART /////////////////
        virtual Identifier imgui_texture_id(const Identifier&) VIRTUAL_METHOD;

        virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset) VIRTUAL_METHOD;
        virtual RHI& draw(size_t vertex_count) VIRTUAL_METHOD;

        virtual RHI& create_shader(Identifier&, const PipelineCreateInfo&) VIRTUAL_METHOD;
        virtual RHI& use_shader(const Identifier&) VIRTUAL_METHOD;

        virtual RHI& swap_buffer() VIRTUAL_METHOD;
        virtual RHI& vsync(bool) VIRTUAL_METHOD;
        virtual bool vsync() VIRTUAL_METHOD;
        virtual bool check_format_support(ColorFormat) VIRTUAL_METHOD;

        virtual RHI& on_window_size_changed() VIRTUAL_METHOD;
        virtual RHI& begin_render() VIRTUAL_METHOD;
        virtual RHI& end_render() VIRTUAL_METHOD;
        virtual RHI& wait_idle() VIRTUAL_METHOD;
        virtual RHI& async_render(bool flag) VIRTUAL_METHOD;
        virtual bool async_render() VIRTUAL_METHOD;
        virtual RHI& next_render_thread() VIRTUAL_METHOD;
        virtual String renderer() VIRTUAL_METHOD;


        virtual RHI_Sampler* create_sampler(const SamplerCreateInfo&)                                     = 0;
        virtual RHI_Texture* create_texture(const TextureCreateInfo&, TextureType type, const byte* data) = 0;

        virtual RHI_FrameBuffer* window_framebuffer()                                  = 0;
        virtual RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info) = 0;

        virtual RHI_Shader* create_vertex_shader(const VertexShader* shader)     = 0;
        virtual RHI_Shader* create_fragment_shader(const FragmentShader* shader) = 0;
        virtual RHI_Pipeline* create_pipeline(const Pipeline* pipeline)          = 0;

        virtual RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data)                = 0;
        virtual RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) = 0;
        virtual RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data)              = 0;
        virtual RHI_SSBO* create_ssbo(size_t size, const byte* data)                                 = 0;


        virtual ~RHI(){};
    };

#undef VIRTUAL_METHOD
}// namespace Engine
