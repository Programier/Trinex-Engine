#include <Core/definitions.hpp>
#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <imgui_impl_opengl3.h>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_render_target.hpp>
#include <opengl_shader.hpp>

namespace Engine
{

    OpenGL* OpenGL::_M_instance = nullptr;

    OpenGL::OpenGL()
    {
        _M_instance = this;
    }

    static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                               const GLchar* message, const void* userParam)
    {
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            error_log("OpenGL", "%s", message);
        }
    }

    void* OpenGL::init_window(struct WindowInterface* window, const WindowConfig& config)
    {
        _M_window  = window;
        _M_surface = window->create_surface("");

#if USING_OPENGL_CORE
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            error_log("OpenGL", "Cannot init glew!");
        }
#endif

        _M_main_render_target = new OpenGL_MainRenderTarget();

        _M_main_render_target->_M_viewport.size      = config.size;
        _M_main_render_target->_M_viewport.pos       = {0.f, 0.f};
        _M_main_render_target->_M_viewport.min_depth = 0.0f;
        _M_main_render_target->_M_viewport.max_depth = 1.0f;
        _M_main_render_target->_M_scissor.pos        = {0.0f, 0.f};
        _M_main_render_target->_M_scissor.size       = config.size;


        // Або встановлення власного обробника повідомлень
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug_callback, nullptr);
        return _M_surface;
    }

    OpenGL& OpenGL::destroy_window()
    {
        delete _M_main_render_target;
        return *this;
    }

    OpenGL& OpenGL::imgui_init()
    {
        ImGui_ImplOpenGL3_Init("#version 300 es");
        return *this;
    }

    OpenGL& OpenGL::imgui_terminate()
    {
        ImGui_ImplOpenGL3_Shutdown();
        return *this;
    }

    OpenGL& OpenGL::imgui_new_frame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        return *this;
    }

    OpenGL& OpenGL::imgui_render()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return *this;
    }

    OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        glDrawElements(_M_current_pipeline->_M_topology, indices_count, _M_current_index_buffer->_M_element_type,
                       reinterpret_cast<void*>(indices_offset));
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw(size_t vertex_count)
    {
        glDrawArrays(_M_current_pipeline->_M_topology, 0, vertex_count);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::vsync(bool flag)
    {
        _M_window->vsync(flag);
        return *this;
    }

    bool OpenGL::vsync()
    {
        return _M_window->vsync();
    }

    OpenGL& OpenGL::on_window_size_changed()
    {
        return *this;
    }

    OpenGL& OpenGL::swap_buffer()
    {
        _M_window->swap_buffers();
        return *this;
    }

    OpenGL& OpenGL::begin_render()
    {
        _M_current_render_target = nullptr;
        _M_current_pipeline      = nullptr;
        _M_current_index_buffer  = nullptr;
        return *this;
    }

    OpenGL& OpenGL::end_render()
    {
        return *this;
    }

    OpenGL& OpenGL::wait_idle()
    {
        return *this;
    }

    String OpenGL::renderer()
    {
        return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    }

    RHI_RenderTarget* OpenGL::window_render_target()
    {
        return _M_main_render_target;
    }

    RHI_RenderPass* OpenGL::window_render_pass()
    {
        return reinterpret_cast<RHI_RenderPass*>(_M_main_render_target->_M_render_pass);
    }

    void OpenGL::push_debug_stage(const char* stage, const Color& color)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, stage);
    }

    void OpenGL::pop_debug_stage()
    {
        glPopDebugGroup();
    }

    void OpenGL::reset_samplers()
    {
        for(BindingIndex unit : _M_sampler_units)
        {
            glBindSampler(unit, 0);
        }

        _M_sampler_units.clear();
    }
}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::RHI*)
{
    return new Engine::OpenGL();
}
