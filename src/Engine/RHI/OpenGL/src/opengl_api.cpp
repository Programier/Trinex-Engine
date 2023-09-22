
#include <Window/config.hpp>
#include <Window/window_interface.hpp>
#include <cstring>
#include <imgui_impl_opengl3.h>
#include <opengl_api.hpp>
#include <opengl_buffer.hpp>
#include <opengl_framebuffer.hpp>
#include <opengl_shader.hpp>
#include <sstream>


#ifdef _WIN32
#define OPENGL_EXPORT __declspec(dllexport) __cdecl
#else
#define OPENGL_EXPORT
#endif

#define API_EXPORT extern "C" OPENGL_EXPORT


namespace Engine
{
    OpenGL* OpenGL::_M_open_gl = nullptr;

    static OpenGL*& get_instance()
    {
        static OpenGL* opengl = nullptr;
        if (opengl == nullptr)
        {
            opengl = new OpenGL();
        }

        return opengl;
    }

    OpenGL::OpenGL()
    {
        _M_open_gl = this;
    }

    OpenGL::~OpenGL()
    {
        _M_open_gl = nullptr;
    }

    bool OpenGL::extension_supported(const String& extension_name)
    {
        static Set<std::string> _M_extentions;

        if (_M_extentions.empty())
        {
            std::istringstream stream(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
            String extension;

            while (stream >> extension) _M_extentions.insert(extension);
        }

        return _M_extentions.contains(extension_name);
    }

    void* OpenGL::init_window(WindowInterface* window, const WindowConfig& config)
    {
        if (_M_context)
            return _M_context;

        window->vsync(config.vsync);

        _M_context = window->create_surface("");
        opengl_debug_log("OpenGL", "Context address: %p\n", _M_context);

#ifdef _WIN32
        info_log("GLEW", "Start init glew");
        auto status = glewInit();
        if (status != GLEW_OK)
        {
            destroy_window();
            opengl_debug_log("OpenGL", "Failed to init glew: %s\n", glewGetErrorString(status));
        }
#endif

        _M_support_anisotropy = !!extension_supported("GL_EXT_texture_filter_anisotropic");
        _M_window_interface   = window;
        return _M_context;
    }

    OpenGL& OpenGL::destroy_object(Identifier& ID)
    {
        delete object_of(ID);
        return *this;
    }

    OpenGL& OpenGL::destroy_window()
    {
        _M_context = nullptr;
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

    OpenGL& OpenGL::on_window_size_changed()
    {
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

    OpenGL& OpenGL::async_render(bool flag)
    {
        return *this;
    }

    bool OpenGL::async_render()
    {
        return false;
    }

    OpenGL& OpenGL::next_render_thread()
    {
        return *this;
    }

    String OpenGL::renderer()
    {
        return String(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    }

    OpenGL& OpenGL::create_vertex_buffer(Identifier& ID, const byte* data, size_t size)
    {
        ID = (new OpenGL_VertexBuffer())->create(data, size).ID();
        return *this;
    }

    OpenGL& OpenGL::update_vertex_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        GET_TYPE(OpenGL_VertexBuffer, ID)->update(offset, data, size);
        return *this;
    }

    OpenGL& OpenGL::bind_vertex_buffer(const Identifier& ID, size_t offset)
    {
        GET_TYPE(OpenGL_VertexBuffer, ID)->bind(offset);
        return *this;
    }

    OpenGL& OpenGL::create_index_buffer(Identifier& ID, const byte* data, size_t size, IndexBufferComponent component)
    {
        ID = (new OpenGL_IndexBuffer())->component_type(component).create(data, size).ID();
        return *this;
    }

    OpenGL& OpenGL::update_index_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        GET_TYPE(OpenGL_IndexBuffer, ID)->update(offset, data, size);
        return *this;
    }

    OpenGL& OpenGL::bind_index_buffer(const Identifier& ID, size_t offset)
    {
        GET_TYPE(OpenGL_IndexBuffer, ID)->bind(offset);
        return *this;
    }

    OpenGL& OpenGL::create_uniform_buffer(Identifier& ID, const byte* data, size_t size)
    {
        ID = (new OpenGL_UniformBufferMap(data, size))->ID();
        return *this;
    }

    OpenGL& OpenGL::update_uniform_buffer(const Identifier& ID, size_t offset, const byte* data, size_t size)
    {
        GET_TYPE(OpenGL_UniformBufferMap, ID)->current_buffer()->update(offset, data, size);
        return *this;
    }

    OpenGL& OpenGL::bind_uniform_buffer(const Identifier& ID, BindingIndex binding)
    {
        GET_TYPE(OpenGL_UniformBufferMap, ID)->current_buffer()->bind(binding);
        return *this;
    }

    MappedMemory OpenGL::map_uniform_buffer(const Identifier& ID)
    {
        return GET_TYPE(OpenGL_UniformBufferMap, ID)->current_buffer()->map_memory();
    }

    OpenGL& OpenGL::unmap_uniform_buffer(const Identifier& ID)
    {
        GET_TYPE(OpenGL_UniformBufferMap, ID)->current_buffer()->unmap_memory();
        return *this;
    }

    OpenGL& OpenGL::draw_indexed(size_t indices_count, size_t indices_offset)
    {
        glDrawElements(state.shader->_M_topology, indices_count, state.index_buffer->_M_component_type,
                       reinterpret_cast<void*>(indices_offset));

        for (auto& unit : _M_samplers)
        {
            glBindSampler(unit, 0);
        }

        _M_samplers.clear();
        return *this;
    }

    OpenGL& OpenGL::swap_buffer()
    {
        _M_window_interface->swap_buffers();
        _M_current_buffer_index = (_M_current_buffer_index + 1) % 2;
        _M_next_buffer_index    = (_M_next_buffer_index + 1) % 2;
        return *this;
    }

    OpenGL& OpenGL::vsync(bool)
    {
        return *this;
    }

    bool OpenGL::vsync()
    {
        return false;
    }

    bool OpenGL::check_format_support(ColorFormat)
    {
        throw std::runtime_error(not_implemented);
    }
}// namespace Engine


API_EXPORT Engine::RHI* load_api()
{
    return Engine::get_instance();
}
