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

    NoApi& NoApi::create_vertex_buffer(Identifier&, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_vertex_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_vertex_buffer(const Identifier&, size_t offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_vertex_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_vertex_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_index_buffer(Identifier&, const byte*, size_t, IndexBufferComponent)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_index_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::bind_index_buffer(const Identifier&, size_t offset)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    MappedMemory NoApi::map_index_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }
    NoApi& NoApi::unmap_index_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::create_uniform_buffer(Identifier&, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_uniform_buffer(const Identifier&, size_t offset, const byte*, size_t)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_uniform_buffer(const Identifier&, BindingIndex binding)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    MappedMemory NoApi::map_uniform_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::unmap_uniform_buffer(const Identifier& ID)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::draw_indexed(size_t indices_count, size_t indices_offset)
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

    NoApi& NoApi::create_ssbo(Identifier&, const byte* data, size_t size)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::bind_ssbo(const Identifier&, BindingIndex index, size_t offset, size_t size)
    {
        error_log("NoApi", "Function '%s' is no implemented!", __PRETTY_FUNCTION__);
        return *this;
    }

    NoApi& NoApi::update_ssbo(const Identifier&, const byte*, size_t offset, size_t size)
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

}// namespace Engine
