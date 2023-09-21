#pragma once
#include <api.hpp>

namespace Engine
{
    class NoApi : public RHI::ApiInterface
    {
        void* init_window(WindowInterface*, const WindowConfig&) override;
        NoApi& destroy_window() override;
        NoApi& destroy_object(Identifier&) override;
        NoApi& imgui_init() override;
        NoApi& imgui_terminate() override;
        NoApi& imgui_new_frame() override;
        NoApi& imgui_render() override;

        ///////////////// TEXTURE PART /////////////////
        Identifier imgui_texture_id(const Identifier&) override;
        NoApi& create_vertex_buffer(Identifier&, const byte*, size_t) override;
        NoApi& update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_vertex_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_vertex_buffer(const Identifier& ID) override;
        NoApi& unmap_vertex_buffer(const Identifier& ID) override;

        NoApi& create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent) override;
        NoApi& update_index_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_index_buffer(const Identifier&, size_t offset) override;
        MappedMemory map_index_buffer(const Identifier& ID) override;
        NoApi& unmap_index_buffer(const Identifier& ID) override;

        NoApi& create_uniform_buffer(Identifier&, const byte*, size_t) override;
        NoApi& update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t) override;
        NoApi& bind_uniform_buffer(const Identifier&, BindingIndex binding) override;
        MappedMemory map_uniform_buffer(const Identifier& ID) override;
        NoApi& unmap_uniform_buffer(const Identifier& ID) override;

        NoApi& draw_indexed(size_t indices_count, size_t indices_offset) override;

        NoApi& create_shader(Identifier&, const PipelineCreateInfo&) override;
        NoApi& use_shader(const Identifier&) override;

        NoApi& create_ssbo(Identifier&, const byte* data, size_t size) override;
        NoApi& bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size) override;
        NoApi& update_ssbo(const Identifier&, const byte*, size_t offset, size_t size) override;

        NoApi& swap_buffer() override;
        NoApi& vsync(bool) override;
        bool vsync() override;
        bool check_format_support(ColorFormat) override;

        NoApi& on_window_size_changed() override;
        NoApi& begin_render() override;
        NoApi& end_render() override;
        NoApi& wait_idle() override;
        NoApi& async_render(bool flag) override;
        bool async_render() override;
        NoApi& next_render_thread() override;
        String renderer() override;


        RHI::RHI_Sampler* create_sampler(const SamplerCreateInfo&) override;
        RHI::RHI_Texture* create_texture(const TextureCreateInfo&, TextureType type, const byte* data) override;
        RHI::RHI_FrameBuffer* window_framebuffer() override;
        RHI::RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info) override;


        template<typename Type>
        operator Type()
        {
            return Type();
        }
    };
}// namespace Engine
