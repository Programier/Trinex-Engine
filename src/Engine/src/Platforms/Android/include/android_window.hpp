#pragma once
#include <EGL/egl.h>
#include <Window/window_interface.hpp>

struct ANativeWindow;
struct AInputEvent;
struct ImGuiContext;

namespace Engine
{
    class ENGINE_EXPORT AndroidWindow : public WindowInterface
    {
    protected:
        String m_name;
        bool m_is_resizable : 1 = true;
        bool m_init_vsync : 1   = true;

    public:
        struct ImGuiContext* imgui_context = nullptr;

        AndroidWindow(const WindowConfig* config);

        static ANativeWindow* native_window();

        String title() override;
        WindowInterface& title(const String& title) override;
        Point2D position() override;
        WindowInterface& position(const Point2D& position) override;
        Size1D width() override;
        WindowInterface& width(const Size1D& width) override;
        Size1D height() override;
        WindowInterface& height(const Size1D& height) override;
        Size2D size() override;
        WindowInterface& size(const Size2D& size) override;
        bool resizable() override;
        WindowInterface& resizable(bool value) override;
        Identifier id() override;

        WindowInterface& focus() override;
        bool focused() override;
        WindowInterface& show() override;
        WindowInterface& hide() override;
        bool is_visible() override;
        bool is_iconify() override;
        WindowInterface& iconify() override;
        bool is_restored() override;
        WindowInterface& restore() override;
        WindowInterface& opacity(float value) override;
        float opacity() override;

        WindowInterface& window_icon(const Image& image) override;
        WindowInterface& cursor(const Image& image, IntVector2D hotspot = {0, 0}) override;
        WindowInterface& attribute(const WindowAttribute& attrib, bool value) override;
        bool attribute(const WindowAttribute& attrib) override;
        WindowInterface& cursor_mode(const CursorMode& mode) override;
        CursorMode cursor_mode() override;
        bool support_orientation(WindowOrientation orientation) override;

        WindowInterface& initialize_imgui() override;
        WindowInterface& terminate_imgui() override;
        WindowInterface& new_imgui_frame() override;
        int32_t process_imgui_event(AInputEvent* event);

        ~AndroidWindow();
    };


    class ENGINE_EXPORT AndroidEGLWindow : public AndroidWindow
    {
    public:
        EGLDisplay egl_display = EGL_NO_DISPLAY;
        EGLSurface egl_surface = EGL_NO_SURFACE;
        EGLContext egl_context = EGL_NO_CONTEXT;

        AndroidEGLWindow(const WindowConfig* config);

        virtual void* create_api_context(const char* any_text, ...) override;
        virtual void bind_api_context(void* context) override;

        WindowInterface& make_current() override;
        WindowInterface& destroy_api_context() override;
        WindowInterface& vsync(bool) override;
        WindowInterface& present() override;
        bool vsync() override;
        Vector<String> required_extensions() override;
    };

    class ENGINE_EXPORT AndroidVulkanWindow : public AndroidWindow
    {
    public:
        AndroidVulkanWindow(const WindowConfig* config);

        void* create_api_context(const char* any_text, ...) override;
        void bind_api_context(void* context) override;

        WindowInterface& make_current() override;
        WindowInterface& destroy_api_context() override;
        WindowInterface& vsync(bool) override;
        WindowInterface& present() override;
        bool vsync() override;
        Vector<String> required_extensions() override;
    };
}// namespace Engine
