#include <Core/exception.hpp>
#include <EGL/egl.h>
#include <Window/config.hpp>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <android_platform.hpp>
#include <android_window.hpp>
#include <imgui_impl_android.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

namespace Engine
{
    AndroidWindow::AndroidWindow(const WindowConfig* config) : m_name("Android Window")
    {
        ANativeWindow_release(native_window());
        m_is_resizable = config->contains_attribute(WindowAttribute::Resizable);
        m_init_vsync   = config->vsync;
    }

    AndroidWindow::~AndroidWindow()
    {
        ANativeWindow_release(native_window());
    }

    ANativeWindow* AndroidWindow::native_window()
    {
        return Platform::android_application()->window;
    }

    String AndroidWindow::title()
    {
        return m_name;
    }

    WindowInterface& AndroidWindow::title(const String& title)
    {
        m_name = title;
        return *this;
    }

    Point2D AndroidWindow::position()
    {
        return {0.f, 0.f};
    }

    WindowInterface& AndroidWindow::position(const Point2D& position)
    {
        return *this;
    }

    Size1D AndroidWindow::width()
    {
        ANativeWindow* window = native_window();
        return static_cast<Size1D>(ANativeWindow_getWidth(window));
    }

    WindowInterface& AndroidWindow::width(const Size1D& value)
    {
        size(Size2D(value, height()));
        return *this;
    }

    Size1D AndroidWindow::height()
    {
        ANativeWindow* window = native_window();
        return static_cast<Size1D>(ANativeWindow_getHeight(window));
    }

    WindowInterface& AndroidWindow::height(const Size1D& value)
    {
        size(Size2D(height(), value));
        return *this;
    }

    Size2D AndroidWindow::size()
    {
        return Size2D(width(), height());
    }

    WindowInterface& AndroidWindow::size(const Size2D& size)
    {
        if (resizable())
        {
            uint32_t x = static_cast<uint32_t>(size.x);
            uint32_t y = static_cast<uint32_t>(size.y);

            ANativeWindow* window = native_window();
            ANativeWindow_setBuffersGeometry(window, x, y, ANativeWindow_getFormat(window));
        }
        return *this;
    }

    bool AndroidWindow::resizable()
    {
        return m_is_resizable;
    }

    WindowInterface& AndroidWindow::resizable(bool value)
    {
        m_is_resizable = value;
        return *this;
    }

    Identifier AndroidWindow::id()
    {
        return reinterpret_cast<Identifier>(native_window());
    }


    WindowInterface& AndroidWindow::focus()
    {
        return *this;
    }

    bool AndroidWindow::focused()
    {
        return true;
    }

    WindowInterface& AndroidWindow::show()
    {
        return *this;
    }

    WindowInterface& AndroidWindow::hide()
    {
        return *this;
    }

    bool AndroidWindow::is_visible()
    {
        return true;
    }

    bool AndroidWindow::is_iconify()
    {
        return false;
    }

    WindowInterface& AndroidWindow::iconify()
    {
        return *this;
    }

    bool AndroidWindow::is_restored()
    {
        return true;
    }

    WindowInterface& AndroidWindow::restore()
    {
        return *this;
    }

    WindowInterface& AndroidWindow::opacity(float value)
    {
        return *this;
    }

    float AndroidWindow::opacity()
    {
        return 1.f;
    }

    WindowInterface& AndroidWindow::window_icon(const Image& image)
    {
        return *this;
    }

    WindowInterface& AndroidWindow::cursor(const Image& image, IntVector2D hotspot)
    {
        return *this;
    }

    WindowInterface& AndroidWindow::attribute(const WindowAttribute& attrib, bool value)
    {
        return *this;
    }

    bool AndroidWindow::attribute(const WindowAttribute& attrib)
    {
        return false;
    }

    WindowInterface& AndroidWindow::cursor_mode(const CursorMode& mode)
    {
        return *this;
    }

    CursorMode AndroidWindow::cursor_mode()
    {
        return CursorMode::Normal;
    }

    bool AndroidWindow::support_orientation(WindowOrientation orientation)
    {
        return true;
    }


    struct ImGuiContextSaver {
        ImGuiContext* context;

        ImGuiContextSaver() : context(ImGui::GetCurrentContext())
        {}

        ImGuiContextSaver(ImGuiContext* new_context) : context(ImGui::GetCurrentContext())
        {
            ImGui::SetCurrentContext(new_context);
        }

        ~ImGuiContextSaver()
        {
            ImGui::SetCurrentContext(context);
        }
    };

    WindowInterface& AndroidWindow::initialize_imgui()
    {
        if (!imgui_context)
        {
            ImGuiContextSaver saver;

            ImGui_ImplAndroid_Init(native_window());
            imgui_context = ImGui::GetCurrentContext();
        }
        return *this;
    }

    WindowInterface& AndroidWindow::terminate_imgui()
    {
        if (imgui_context)
        {
            ImGuiContextSaver saver(imgui_context);
            ImGui_ImplAndroid_Shutdown();
        }
        return *this;
    }

    WindowInterface& AndroidWindow::new_imgui_frame()
    {
        if (imgui_context)
        {
            ImGui_ImplAndroid_NewFrame();
        }
        return *this;
    }

    int32_t AndroidWindow::process_imgui_event(AInputEvent* event)
    {
        if (imgui_context)
        {
            ImGuiContextSaver saver(imgui_context);
            ImGui_ImplAndroid_HandleInputEvent(event);
        }
        return 0;
    }

    //// EGL WINDOW

    AndroidEGLWindow::AndroidEGLWindow(const WindowConfig* config) : AndroidWindow(config)
    {
        egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        if (egl_display == EGL_NO_DISPLAY)
        {
            throw EngineException("eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");
        }

        if (eglInitialize(egl_display, 0, 0) != EGL_TRUE)
        {
            throw EngineException("eglInitialize() returned with an error");
        }
    }

    void* AndroidEGLWindow::create_api_context(const char* any_text, ...)
    {
        const EGLint egl_attributes[] = {
                EGL_BLUE_SIZE,  8,       EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE,
                EGL_WINDOW_BIT, EGL_NONE};

        EGLint num_configs = 0;

        if (eglChooseConfig(egl_display, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE)
            throw EngineException("eglChooseConfig() returned with an error");

        if (num_configs == 0)
            throw EngineException("eglChooseConfig() returned 0 matching config");

        // Get the first matching config

        EGLConfig egl_config;
        eglChooseConfig(egl_display, egl_attributes, &egl_config, 1, &num_configs);
        EGLint egl_format;

        eglGetConfigAttrib(egl_display, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
        ANativeWindow_setBuffersGeometry(native_window(), width(), height(), egl_format);

        const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        egl_context                           = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, egl_context_attributes);

        if (egl_context == EGL_NO_CONTEXT)
            throw EngineException("eglCreateContext() returned EGL_NO_CONTEXT");

        egl_surface = eglCreateWindowSurface(egl_display, egl_config, native_window(), nullptr);
        eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

        vsync(m_init_vsync);
        return egl_context;
    }

    void AndroidEGLWindow::bind_api_context(void* context)
    {
        // Android can create only one window instance
    }

    WindowInterface& AndroidEGLWindow::make_current()
    {
        eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
        return *this;
    }

    WindowInterface& AndroidEGLWindow::destroy_api_context()
    {
        if (egl_display != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (egl_context != EGL_NO_CONTEXT)
                eglDestroyContext(egl_display, egl_context);

            if (egl_surface != EGL_NO_SURFACE)
                eglDestroySurface(egl_display, egl_surface);

            eglTerminate(egl_display);

            egl_display = EGL_NO_DISPLAY;
            egl_surface = EGL_NO_SURFACE;
            egl_context = EGL_NO_CONTEXT;
        }
        return *this;
    }

    WindowInterface& AndroidEGLWindow::vsync(bool vsync_value)
    {
        m_init_vsync = vsync_value;
        eglSwapInterval(egl_display, vsync_value ? 1 : 0);
        return *this;
    }

    WindowInterface& AndroidEGLWindow::present()
    {
        eglSwapBuffers(egl_display, egl_surface);
        return *this;
    }

    bool AndroidEGLWindow::vsync()
    {
        return m_init_vsync;
    }

    Vector<String> AndroidEGLWindow::required_extensions()
    {
        return {};
    }

    // Vulkan Window

    AndroidVulkanWindow::AndroidVulkanWindow(const WindowConfig* config) : AndroidWindow(config)
    {}

    void* AndroidVulkanWindow::create_api_context(const char* any_text, ...)
    {
        va_list args;
        va_start(args, any_text);
        VkInstance instance = va_arg(args, VkInstance);
        va_end(args);

        ANativeWindow* window = native_window();
        VkSurfaceKHR surface;
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType                         = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window                        = window;

        if (vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
        {
            throw EngineException("Android: Failed to create Vulkan Surface");
        }

        return reinterpret_cast<void*>(surface);
    }

    void AndroidVulkanWindow::bind_api_context(void* context)
    {
        // Android can use only one window, so this method does not affect
    }

    WindowInterface& AndroidVulkanWindow::make_current()
    {
        return *this;// Must never call
    }

    WindowInterface& AndroidVulkanWindow::destroy_api_context()
    {
        return *this;// Must never call.
    }

    WindowInterface& AndroidVulkanWindow::vsync(bool)
    {
        return *this;// Must never call
    }

    WindowInterface& AndroidVulkanWindow::present()
    {
        return *this;// Must never call
    }

    bool AndroidVulkanWindow::vsync()
    {
        return false;// Must never call
    }

    Vector<String> AndroidVulkanWindow::required_extensions()
    {
        return {"VK_KHR_surface", "VK_KHR_android_surface"};
    }
}// namespace Engine
