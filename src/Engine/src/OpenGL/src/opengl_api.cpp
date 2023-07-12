#include <SDL.h>
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

    OpenGL& OpenGL::logger(Logger*& logger)
    {
        _M_logger = &logger;
        return *this;
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

    void* OpenGL::init_window(SDL_Window* window)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef _WIN32
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

        if (_M_context)
            return _M_context;

        _M_context = SDL_GL_CreateContext(window);

        if (!_M_context)
        {
            opengl_debug_log("OpenGL", "Failed to create OpenGL context: %s\n", SDL_GetError());
            return nullptr;
        }

        SDL_GL_MakeCurrent(window, _M_context);
        opengl_debug_log("OpenGL", "Context address: %p\n", _M_context);

#ifdef _WIN32
        (*_M_logger)->log("GLEW", "Start init glew\n");
        auto status = glewInit();
        if (status != GLEW_OK)
        {
            destroy_window();
            opengl_debug_log("OpenGL", "Failed to init glew: %s\n", glewGetErrorString(status));
        }
#endif

        _M_support_anisotropy = !!extension_supported("GL_EXT_texture_filter_anisotropic");
        return _M_context;
    }

    OpenGL& OpenGL::destroy_object(Identifier& ID)
    {
        delete object_of(ID);
        return *this;
    }

    OpenGL& OpenGL::destroy_window()
    {
        if (_M_context)
            SDL_GL_DeleteContext(_M_context);
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
        glDrawElements(_M_current_shader->_M_topology, indices_count, _M_index_buffer->_M_component_type,
                       reinterpret_cast<void*>(indices_offset));
        return *this;
    }


    OpenGL& OpenGL::gen_framebuffer(Identifier& ID, const FrameBufferCreateInfo& info)
    {
        ID = (new OpenGL_FrameBufferSet())->gen_framebuffer(info).ID();
        return *this;
    }

    OpenGL_FrameBufferSet* OpenGL::framebuffer(Identifier ID)
    {
        static OpenGL_FrameBufferSet base_framebuffer(true);
        if (ID)
        {
            return GET_TYPE(OpenGL_FrameBufferSet, ID);
        }
        return &base_framebuffer;
    }

    OpenGL& OpenGL::bind_framebuffer(const Identifier& ID, size_t buffer_index)
    {
        framebuffer(ID)->bind_framebuffer((ID == 0 ? 0 : buffer_index));
        return *this;
    }

    OpenGL& OpenGL::framebuffer_viewport(const Identifier& ID, const ViewPort& viewport)
    {
        framebuffer(ID)->framebuffer_viewport(viewport);
        return *this;
    }

    OpenGL& OpenGL::framebuffer_scissor(const Identifier& ID, const Scissor& scissor)
    {
        framebuffer(ID)->framebuffer_scissor(scissor);
        return *this;
    }

    OpenGL& OpenGL::swap_buffer(SDL_Window* window)
    {
        SDL_GL_SwapWindow(window);
        _M_current_buffer_index = (_M_current_buffer_index + 1) % 2;
        _M_next_buffer_index    = (_M_next_buffer_index + 1) % 2;
        return *this;
    }

    OpenGL& OpenGL::swap_interval(int_t interval)
    {
        SDL_GL_SetSwapInterval(interval);
        return *this;
    }

    OpenGL& OpenGL::clear_color(const Identifier&, const ColorClearValue&, byte layout)
    {
        throw std::runtime_error(not_implemented);
    }

    OpenGL& OpenGL::clear_depth_stencil(const Identifier&, const DepthStencilClearValue&)
    {
        throw std::runtime_error(not_implemented);
    }

    bool OpenGL::check_format_support(PixelType type, PixelComponentType component)
    {
        throw std::runtime_error(not_implemented);
    }
}// namespace Engine


API_EXPORT Engine::GraphicApiInterface::ApiInterface* load_api()
{
    return Engine::get_instance();
}
