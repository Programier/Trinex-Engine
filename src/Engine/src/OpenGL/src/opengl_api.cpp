#ifdef _WIN32
#include <GL/glew.h>
#endif
#include <Core/logger.hpp>
#include <SDL.h>
#include <fstream>
#include <opengl_api.hpp>
#include <opengl_export.hpp>
#include <opengl_types.hpp>


namespace Engine
{
    class NoneLogger : public Logger
    {
        Logger& log(const char* format, ...) override
        {
            return *this;
        }
    };

    void* OpenGL_Object::instance_address()
    {
        return reinterpret_cast<void*>(this);
    }

    OpenGL_Object::~OpenGL_Object()
    {}

    static Logger* default_logger()
    {
        static NoneLogger logger;
        return &logger;
    }

    OpenGL_Object* OpenGL::instance(const ObjID& ID)
    {
        return reinterpret_cast<OpenGL_Object*>(ID);
    }

    //////////////////// INITIALIZE API ////////////////////
    OpenGL::OpenGL()
    {
        _M_current_logger = default_logger();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#ifdef _WIN32
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    }


    //////////////////// TERMINATE API ////////////////////

    OpenGL::~OpenGL()
    {
        _M_current_logger->log("Terminate OpenGL\n");
        _M_api = nullptr;
    }

    //////////////////// API IMPLEMENTATION ////////////////////
    void* OpenGL::OpenGL::init_window(SDL_Window* window)
    {
        if (_M_context)
            return _M_context;
        _M_context = SDL_GL_CreateContext(window);

        if (!_M_context)
        {
            _M_current_logger->log("Failed to create OpenGL context: %s\n", SDL_GetError());
            return nullptr;
        }

        SDL_GL_MakeCurrent(window, _M_context);
        _M_current_logger->log("Context address: %p\n", _M_context);

#ifdef _WIN32
        external_logger->log("Start init glew\n");
        auto status = glewInit();
        if (status != GLEW_OK)
        {
            _M_api->destroy_window();
            _M_current_logger->log("Failed to init glew: %s\n", glewGetErrorString(status));
        }
#endif

        return _M_context;
    }

    OpenGL& OpenGL::logger(Logger* logger)
    {
        if (!logger)
            logger = default_logger();

        _M_current_logger = logger;
        return *this;
    }

    OpenGL* OpenGL::_M_api = nullptr;

    OpenGL* OpenGL::instance()
    {
        if (_M_api == nullptr)
            _M_api = new OpenGL();
        return _M_api;
    }

    API_EXPORT Engine::GraphicApiInterface::ApiInterface* load_api()
    {
        Engine::GraphicApiInterface::ApiInterface* result = Engine::OpenGL::instance();
        return result;
    }

    ObjID OpenGL::get_object_id(OpenGL_Object* object)
    {
        return reinterpret_cast<ObjID>(object);
    }

    int_t OpenGL::get_current_binding(GLint type)
    {
        int value;
        glGetIntegerv(type, &value);
        return value;
    }


    OpenGL& OpenGL::destroy_window()
    {
        if (_M_context)
            SDL_GL_DeleteContext(_M_context);
        _M_context = nullptr;
        return *this;
    }

    float OpenGL::line_rendering_width()
    {
        return _M_current_line_rendering_width;
    }

    OpenGL& OpenGL::line_rendering_width(float value)
    {
        _M_current_line_rendering_width = value < 0 ? -value : value;
        glLineWidth(_M_current_line_rendering_width);
        return *this;
    }

    OpenGL& OpenGL::destroy_object(ObjID& ID)
    {
        delete instance(ID);
        ID = 0;
        return *this;
    }

    OpenGL& OpenGL::enable(Engine::EnableCap cap)
    {
        glEnable(_M_enable_caps.at(cap));
        return *this;
    }

    OpenGL& OpenGL::disable(Engine::EnableCap cap)
    {
        glDisable(_M_enable_caps.at(cap));
        return *this;
    }

    OpenGL& OpenGL::blend_func(Engine::BlendFunc func1, Engine::BlendFunc func2)
    {
        glBlendFunc(_M_blend_funcs.at(func1), _M_blend_funcs.at(func2));
        return *this;
    }

    OpenGL& OpenGL::depth_func(Engine::DepthFunc func)
    {
        glDepthFunc(_M_compare_funcs.at(func));
        return *this;
    }

    OpenGL& OpenGL::depth_mask(bool flag)
    {
        glDepthMask(static_cast<GLboolean>(flag));
        return *this;
    }

    OpenGL& OpenGL::stencil_mask(byte mask)
    {
        glStencilMask(mask);
        return *this;
    }

    OpenGL& OpenGL::stencil_func(Engine::CompareFunc func, int_t ref, byte mask)
    {
        glStencilFunc(_M_compare_funcs.at(func), ref, mask);
        return *this;
    }

    OpenGL& OpenGL::stencil_option(Engine::StencilOption stencil_fail, Engine::StencilOption depth_fail,
                                   Engine::StencilOption pass)
    {
        glStencilOp(_M_stencil_options.at(stencil_fail), _M_stencil_options.at(depth_fail),
                    _M_stencil_options.at(pass));
        return *this;
    }


    OpenGL& OpenGL::swap_buffer(SDL_Window* window)
    {
        SDL_GL_SwapWindow(window);
        return *this;
    }

    OpenGL& OpenGL::swap_interval(uint_t value)
    {
        SDL_GL_SetSwapInterval(value);
        return *this;
    }

    OpenGL& OpenGL::clear_color(const Color& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        return *this;
    }

}// namespace Engine
