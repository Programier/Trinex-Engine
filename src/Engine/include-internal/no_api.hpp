#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
    class NoApi : public RHI
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

        NoApi& draw_indexed(size_t indices_count, size_t indices_offset) override;
        NoApi& draw(size_t vertex_count) override;

        NoApi& create_shader(Identifier&, const PipelineCreateInfo&) override;
        NoApi& use_shader(const Identifier&) override;

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


        RHI_Sampler* create_sampler(const SamplerCreateInfo&) override;
        RHI_Texture* create_texture(const TextureCreateInfo&, TextureType type, const byte* data) override;
        RHI_FrameBuffer* window_framebuffer() override;
        RHI_FrameBuffer* create_framebuffer(const FrameBufferCreateInfo& info) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_UniformBuffer* create_uniform_buffer(size_t size, const byte* data) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;

        template<typename Type>
        operator Type()
        {
            return Type();
        }
    };
}// namespace Engine
