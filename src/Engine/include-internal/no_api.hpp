#pragma once
#include <Core/color_format.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
    class NoApi : public RHI
    {
    public:
        NoApi& imgui_init(ImGuiContext*) override;
        NoApi& imgui_terminate(ImGuiContext*) override;
        NoApi& imgui_new_frame(ImGuiContext*) override;
        NoApi& imgui_render(ImGuiContext*, ImDrawData*) override;
        RHI_ImGuiTexture* imgui_create_texture(ImGuiContext*, Texture* texture, Sampler* sampler) override;

        NoApi& destroy_object(RHI_Object*) override;

        NoApi& draw_indexed(size_t indices_count, size_t indices_offset) override;
        NoApi& draw(size_t vertex_count) override;

        NoApi& begin_render() override;
        NoApi& end_render() override;
        NoApi& wait_idle() override;
        const String& renderer() override;
        const String& name() override;

        RHI_Sampler* create_sampler(const Sampler* sampler) override;
        RHI_Texture* create_texture(const Texture*, const byte* data) override;
        RHI_RenderTarget* create_render_target(const RenderTarget* render_target) override;
        RHI_Shader* create_vertex_shader(const VertexShader* shader) override;
        RHI_Shader* create_fragment_shader(const FragmentShader* shader) override;
        RHI_Pipeline* create_pipeline(const Pipeline* pipeline) override;
        RHI_VertexBuffer* create_vertex_buffer(size_t size, const byte* data) override;
        RHI_IndexBuffer* create_index_buffer(size_t, const byte* data, IndexBufferComponent) override;
        RHI_SSBO* create_ssbo(size_t size, const byte* data) override;
        RHI_RenderPass* create_render_pass(const RenderPass* render_pass) override;
        RHI_RenderPass* window_render_pass() override;
        ColorFormatFeatures color_format_features(ColorFormat) override;
        size_t render_target_buffer_count() override;
        RHI_Viewport* create_viewport(WindowInterface* interface, bool vsync) override;
        RHI_Viewport* create_viewport(RenderTarget* render_target) override;

        NoApi& push_global_params(const GlobalShaderParameters& params) override;
        NoApi& pop_global_params() override;
        NoApi& update_local_parameter(const void* data, size_t size, size_t offset) override;

        void line_width(float width) override;

        void push_debug_stage(const char* stage, const Color& color) override;
        void pop_debug_stage() override;

        template<typename Type>
        operator Type()
        {
            return Type();
        }
    };
}// namespace Engine
