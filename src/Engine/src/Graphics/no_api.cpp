#include <Core/logger.hpp>
#include <no_api.hpp>

namespace Engine
{
    NoApi& NoApi::initialize(Window* window)
    {
        return *this;
    }

    void* NoApi::context()
    {
        return nullptr;
    }

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

    NoApi& NoApi::destroy_object(RHI_Object*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw(size_t vertex_count, size_t vertices_offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances)
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

    void NoApi::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    void NoApi::viewport(const ViewPort& viewport)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    ViewPort NoApi::viewport()
    {
        return {};
    }

    RHI_Sampler* NoApi::create_sampler(const Sampler*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Texture* NoApi::create_texture_2d(const Texture2D*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_vertex_shader(const VertexShader* shader)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_tesselation_control_shader(const TessellationControlShader* shader)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_tesselation_shader(const TessellationShader* shader)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Shader* NoApi::create_geometry_shader(const GeometryShader* shader)
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

    RHI_VertexBuffer* NoApi::create_vertex_buffer(size_t size, const byte* data, RHIBufferType)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_IndexBuffer* NoApi::create_index_buffer(size_t, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_SSBO* NoApi::create_ssbo(size_t size, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    void NoApi::push_debug_stage(const char* stage, const Color& color)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    void NoApi::pop_debug_stage()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
    }

    RHI_Viewport* NoApi::create_viewport(RenderViewport* vp)
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
}// namespace Engine
