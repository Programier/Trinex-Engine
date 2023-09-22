#pragma once


#include <Core/color.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/rhi_initializers.hpp>


namespace Engine
{
    class Logger;
    struct WindowInterface;
    struct WindowConfig;
    struct SamplerCreateInfo;
}// namespace Engine

#define VIRTUAL_METHOD = 0

namespace Engine
{
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

        virtual RHI& create_vertex_buffer(Identifier&, const byte*, size_t) VIRTUAL_METHOD;
        virtual RHI& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual RHI& bind_vertex_buffer(const Identifier&, size_t offset) VIRTUAL_METHOD;
        virtual MappedMemory map_vertex_buffer(const Identifier& ID) VIRTUAL_METHOD;
        virtual RHI& unmap_vertex_buffer(const Identifier& ID) VIRTUAL_METHOD;

        virtual RHI& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) VIRTUAL_METHOD;
        virtual RHI& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual RHI& bind_index_buffer(const Identifier&, size_t offset) VIRTUAL_METHOD;
        virtual MappedMemory map_index_buffer(const Identifier& ID) VIRTUAL_METHOD;
        virtual RHI& unmap_index_buffer(const Identifier& ID) VIRTUAL_METHOD;

        virtual RHI& create_uniform_buffer(Identifier&, const byte*, size_t) VIRTUAL_METHOD;
        virtual RHI& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) VIRTUAL_METHOD;
        virtual RHI& bind_uniform_buffer(const Identifier&, BindingIndex binding) VIRTUAL_METHOD;
        virtual MappedMemory map_uniform_buffer(const Identifier& ID) VIRTUAL_METHOD;
        virtual RHI& unmap_uniform_buffer(const Identifier& ID) VIRTUAL_METHOD;

        virtual RHI& draw_indexed(size_t indices_count, size_t indices_offset) VIRTUAL_METHOD;

        virtual RHI& create_shader(Identifier&, const PipelineCreateInfo&) VIRTUAL_METHOD;
        virtual RHI& use_shader(const Identifier&) VIRTUAL_METHOD;

        virtual RHI& create_ssbo(Identifier&, const byte* data, size_t size) VIRTUAL_METHOD;
        virtual RHI& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) VIRTUAL_METHOD;
        virtual RHI& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) VIRTUAL_METHOD;

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
        virtual RHI_FrameBuffer* window_framebuffer()                                                     = 0;
        virtual RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info)                    = 0;

        virtual ~RHI(){};
    };

#undef VIRTUAL_METHOD
}// namespace Engine
