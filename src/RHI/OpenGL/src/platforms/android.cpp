#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <android_window.hpp>

namespace Engine
{
    struct Context {
        EGLDisplay egl_display = EGL_NO_DISPLAY;
        EGLSurface egl_surface = EGL_NO_SURFACE;
        EGLContext egl_context = EGL_NO_CONTEXT;
        bool m_vsync           = false;
    };

    void* create_opengl_context(Window* main_window)
    {
        AndroidEGLWindow* window = reinterpret_cast<AndroidEGLWindow*>(main_window);
        void* context            = window->create_context();
        window->make_current(context);
        return context;
    }

    void make_window_current(Window* window, void* context)
    {
        reinterpret_cast<AndroidEGLWindow*>(window)->make_current(context);
    }

    bool has_window_vsync(Window* window, void* context)
    {
        return reinterpret_cast<AndroidEGLContext*>(context)->vsync();
    }

    void set_window_vsync(Window* window, void* context, bool flag)
    {
        reinterpret_cast<AndroidEGLContext*>(context)->vsync(flag);
    }

    void swap_window_buffers(Window* window, void* context)
    {
        reinterpret_cast<AndroidEGLWindow*>(window)->swap_buffers(context);
    }

    void destroy_opengl_context(void* context)
    {
        AndroidEGLWindow* window = reinterpret_cast<AndroidEGLWindow*>(WindowManager::instance()->main_window());
        window->destroy_context(context);
    }
}// namespace Engine
