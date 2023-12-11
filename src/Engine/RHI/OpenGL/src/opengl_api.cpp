#include <Core/definitions.hpp>
#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <imgui_impl_opengl3.h>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_render_pass.hpp>
#include <opengl_render_target.hpp>
#include <opengl_shader.hpp>

namespace Engine
{

    OpenGL* OpenGL::_M_instance = nullptr;

    static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                               const GLchar* message, const void* userParam)
    {
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            error_log("OpenGL", "%s", message);
        }
    }


    OpenGL::OpenGL()
    {
        _M_instance         = this;
        _M_main_render_pass = new OpenGL_MainRenderPass();
        _M_main_render_pass->_M_clear_color_attachmend_on_bind.push_back(true);
    }

    OpenGL& OpenGL::initialize()
    {
#if USING_OPENGL_CORE
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            error_log("OpenGL", "Cannot init glew!");
        }
#endif

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug_callback, nullptr);
        return *this;
    }

    OpenGL::~OpenGL()
    {
        delete _M_main_render_pass;
    }


    OpenGL& OpenGL::imgui_init(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplOpenGL3_CreateDeviceObjects();
        return *this;
    }

    OpenGL& OpenGL::imgui_terminate(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_Shutdown();
        return *this;
    }

    OpenGL& OpenGL::imgui_new_frame(ImGuiContext* ctx)
    {
        return *this;
    }

    OpenGL& OpenGL::imgui_render(ImGuiContext* ctx, ImDrawData* draw_data)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_RenderDrawData(draw_data);
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

    OpenGL& OpenGL::destroy_object(RHI_Object* object)
    {
        delete object;
        return *this;
    }


    OpenGL& OpenGL::reset_state()
    {
        _M_current_render_target = nullptr;
        _M_current_pipeline      = nullptr;
        _M_current_index_buffer  = nullptr;
        return *this;
    }

    OpenGL& OpenGL::begin_render()
    {
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


    RHI_RenderPass* OpenGL::window_render_pass()
    {
        return _M_main_render_pass;
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
        for (BindingIndex unit : _M_sampler_units)
        {
            glBindSampler(unit, 0);
        }

        _M_sampler_units.clear();
    }

    size_t OpenGL::render_target_buffer_count()
    {
        return 1;
    }

    void OpenGL::line_width(float width)
    {
        glLineWidth(width);
    }
}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::RHI*)
{
    return new Engine::OpenGL();
}
