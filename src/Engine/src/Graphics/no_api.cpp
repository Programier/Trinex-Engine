#include <Core/logger.hpp>
#include <no_api.hpp>

namespace Engine
{
    NoApi& NoApi::imgui_init(ImGuiContext*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_terminate(ImGuiContext*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_new_frame(ImGuiContext*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_render(ImGuiContext*, ImDrawData*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    RHI_ImGuiTexture* NoApi::imgui_create_texture(ImGuiContext*, Texture* texture, Sampler* sampler)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    NoApi& NoApi::destroy_object(RHI_Object*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw(size_t vertex_count)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::begin_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::end_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::wait_idle()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    const String& NoApi::renderer()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        static const String undefined = "Undefined";
        return undefined;
    }

    const String& NoApi::name()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        static const String noapi = "No API";
        return noapi;
    }

    RHI_Sampler* NoApi::create_sampler(const Sampler*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Texture* NoApi::create_texture(const Texture*, const byte*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_RenderTarget* NoApi::create_render_target(const RenderTarget*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_vertex_shader(const VertexShader* shader)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_fragment_shader(const FragmentShader* shader)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Pipeline* NoApi::create_pipeline(const Pipeline* pipeline)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_VertexBuffer* NoApi::create_vertex_buffer(size_t size, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_IndexBuffer* NoApi::create_index_buffer(size_t, const byte* data, IndexBufferComponent)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_SSBO* NoApi::create_ssbo(size_t size, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_RenderPass* NoApi::create_render_pass(const RenderPass* render_pass)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_RenderPass* NoApi::window_render_pass(RenderPass* engine_render_pass)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    ColorFormatFeatures NoApi::color_format_features(ColorFormat)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return {};
    }

    size_t NoApi::render_target_buffer_count()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return 0;
    }

    void NoApi::push_debug_stage(const char* stage, const Color& color)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    void NoApi::pop_debug_stage()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    RHI_Viewport* NoApi::create_viewport(WindowInterface* interface, bool vsync)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Viewport* NoApi::create_viewport(RenderTarget* render_target)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    NoApi& NoApi::push_global_params(const GlobalShaderParameters& params)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::pop_global_params()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_local_parameter(const void* data, size_t size, size_t offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    ColorFormat NoApi::base_color_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::position_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::normal_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::specular_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::depth_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::stencil_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }

    ColorFormat NoApi::depth_stencil_format()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return ColorFormat::Undefined;
    }


}// namespace Engine
