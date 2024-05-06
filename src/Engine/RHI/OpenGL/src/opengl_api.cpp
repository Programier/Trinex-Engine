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

    OpenGL* OpenGL::m_instance = nullptr;

    static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                               const void* userParam)
    {
        if (type == GL_DEBUG_TYPE_ERROR)
        {
            error_log("OpenGL", "%s", message);
        }
    }

    OpenGL::OpenGL()
    {
        m_instance         = this;
        m_main_render_pass = new OpenGL_MainRenderPass();
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

        m_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        return initialize_ubo();
    }

    OpenGL::~OpenGL()
    {
        delete m_global_ubo;
        delete m_local_ubo;
        delete m_main_render_pass;
    }


    OpenGL& OpenGL::imgui_init(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplOpenGL3_NewFrame();
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

    OpenGL& OpenGL::prepare_render()
    {
        if (m_current_pipeline)
        {
            if (m_current_pipeline->m_global_parameters.has_parameters())
            {
                m_global_ubo->bind(m_current_pipeline->m_global_parameters.bind_index());
            }

            if (m_current_pipeline->m_local_parameters.has_parameters())
            {
                m_local_ubo->bind(m_current_pipeline->m_local_parameters.bind_index());
            }
        }
        return *this;
    }

    OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
    {
        prepare_render();
        glDrawElementsBaseVertex(m_current_pipeline->m_topology, indices_count, GL_UNSIGNED_INT,
                                 reinterpret_cast<void*>(indices_offset), vertices_offset);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw(size_t vertex_count, size_t vertices_offset)
    {
        prepare_render();
        glDrawArrays(m_current_pipeline->m_topology, vertices_offset, vertex_count);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
    {
        prepare_render();
        glDrawArraysInstanced(m_current_pipeline->m_pipeline, vertices_offset, vertex_count, instances);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances)
    {
        prepare_render();
        glDrawElementsInstancedBaseVertex(m_current_pipeline->m_topology, indices_count, GL_UNSIGNED_INT,
                                          reinterpret_cast<void*>(indices_offset), instances, vertices_offset);
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
        m_current_render_target = nullptr;
        m_current_pipeline      = nullptr;
        m_current_index_buffer  = nullptr;
        return *this;
    }

    OpenGL& OpenGL::begin_render()
    {
        m_global_parameters_stack.clear();
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

    const String& OpenGL::renderer()
    {
        return m_renderer;
    }

    const String& OpenGL::name()
    {
        static String api_name =
#if USING_OPENGL_ES
                "OpenGL ES";
#else
                "OpenGL Core";
#endif

        return api_name;
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
        for (BindingIndex unit : m_sampler_units)
        {
            glBindSampler(unit, 0);
        }

        m_sampler_units.clear();
    }
}// namespace Engine


TRINEX_EXTERNAL_LIB_INIT_FUNC(Engine::RHI*)
{
    return new Engine::OpenGL();
}
