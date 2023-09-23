#include <Core/logger.hpp>
#include <no_api.hpp>

namespace Engine
{
    void* NoApi::init_window(WindowInterface*, const WindowConfig&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_window()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::destroy_object(Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_init()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_terminate()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_new_frame()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::imgui_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    Identifier NoApi::imgui_texture_id(const Identifier&)
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

    NoApi& NoApi::create_shader(Identifier&, const PipelineCreateInfo&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::use_shader(const Identifier&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::swap_buffer()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::vsync(bool)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::vsync()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return false;
    }

    bool NoApi::check_format_support(ColorFormat)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::on_window_size_changed()
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

    NoApi& NoApi::async_render(bool flag)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    bool NoApi::async_render()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::next_render_thread()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    String NoApi::renderer()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return "Undefined";
    }

    RHI_Sampler* NoApi::create_sampler(const SamplerCreateInfo&)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_Texture* NoApi::create_texture(const TextureCreateInfo&, TextureType, const byte*)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_FrameBuffer* NoApi::window_framebuffer()
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_FrameBuffer* NoApi::create_framebuffer(const FrameBufferCreateInfo& info)
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

    RHI_UniformBuffer* NoApi::create_uniform_buffer(size_t size, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

    RHI_SSBO* NoApi::create_ssbo(size_t size, const byte* data)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return nullptr;
    }

}// namespace Engine
