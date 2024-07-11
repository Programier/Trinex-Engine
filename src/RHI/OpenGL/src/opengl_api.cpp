#include <Core/definitions.hpp>
#include <Core/struct.hpp>
#include <Window/config.hpp>
#include <imgui_impl_opengl3.h>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_render_target.hpp>
#include <opengl_shader.hpp>

namespace Engine
{
    implement_struct(Engine::RHI, OPENGL, ).push([]() {
        Struct::static_find("Engine::RHI::OPENGL", true)->struct_constructor([]() -> void* {
            if (OpenGL::m_instance == nullptr)
            {
                OpenGL::m_instance                       = new OpenGL();
                OpenGL::m_instance->info.struct_instance = Struct::static_find("Engine::RHI::OPENGL", true);
            }
            return OpenGL::m_instance;
        });
    });

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
#if USING_OPENGL_ES
        info.name = "OpenGL ES";
#else
        info.name = "OpenGL Core";
#endif
    }

    OpenGL::~OpenGL()
    {
        OpenGL_RenderTarget::release_all();
        delete m_global_ubo;
        delete m_local_ubo;

        extern void destroy_opengl_context(void* context);
        destroy_opengl_context(m_context);
    }

    OpenGL& OpenGL::initialize(Window* window)
    {
        extern void* create_opengl_context(Window * window);
        m_context = create_opengl_context(window);

#if USING_OPENGL_CORE
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            error_log("OpenGL", "Cannot init glew!");
        }
#endif

#if TRINEX_DEBUG_BUILD
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debug_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

        info.renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        initialize_ubo();
        return *this;
    }

    void* OpenGL::context()
    {
        return m_context;
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
        if (OPENGL_API->m_state.pipeline)
        {
            if (OPENGL_API->m_state.pipeline->m_global_parameters.has_parameters())
            {
                m_global_ubo->bind(OPENGL_API->m_state.pipeline->m_global_parameters.bind_index());
            }

            if (OPENGL_API->m_state.pipeline->m_local_parameters.has_parameters())
            {
                m_local_ubo->bind(OPENGL_API->m_state.pipeline->m_local_parameters.bind_index());
            }
        }
        return *this;
    }

    OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset, size_t vertices_offset)
    {
        prepare_render();
        indices_offset *= (m_state.index_buffer->m_format == GL_UNSIGNED_SHORT ? 2 : 4);
        glDrawElementsBaseVertex(OPENGL_API->m_state.pipeline->m_topology, indices_count, m_state.index_buffer->m_format,
                                 reinterpret_cast<void*>(indices_offset), vertices_offset);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw(size_t vertex_count, size_t vertices_offset)
    {
        prepare_render();
        glDrawArrays(OPENGL_API->m_state.pipeline->m_topology, vertices_offset, vertex_count);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw_instanced(size_t vertex_count, size_t vertices_offset, size_t instances)
    {
        prepare_render();
        glDrawArraysInstanced(OPENGL_API->m_state.pipeline->m_pipeline, vertices_offset, vertex_count, instances);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::draw_indexed_instanced(size_t indices_count, size_t indices_offset, size_t vertices_offset, size_t instances)
    {
        prepare_render();
        glDrawElementsInstancedBaseVertex(OPENGL_API->m_state.pipeline->m_topology, indices_count, m_state.index_buffer->m_format,
                                          reinterpret_cast<void*>(indices_offset), instances, vertices_offset);
        reset_samplers();
        return *this;
    }

    OpenGL& OpenGL::reset_state()
    {
        new (&m_state) OpenGL_State();
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

    OpenGL& OpenGL::push_debug_stage(const char* stage, const Color& color)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, stage);
        return *this;
    }

    OpenGL& OpenGL::pop_debug_stage()
    {
        glPopDebugGroup();
        return *this;
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
