#pragma once
#include <Core/callback.hpp>
#include <Core/pointer.hpp>
#include <Core/structures.hpp>

struct ImGuiContext;
struct ImDrawData;

namespace Engine
{
    namespace ImGuiRenderer
    {
        class Window;
    }

    struct WindowConfig;
    class Image;

    class ENGINE_EXPORT Window
    {
    public:
        using DestroyCallback = CallBack<void()>;

    private:
        Pointer<class RenderViewport> m_render_viewport;
        Size2D m_cached_size;
        Window* m_parent_window = nullptr;
        Vector<Window*> m_childs;
        Pointer<ImGuiRenderer::Window> m_imgui_window = nullptr;
        CallBacks<void()> m_destroy_callback;

    protected:
        virtual Window& imgui_initialize_internal();
        virtual Window& imgui_terminate_internal();

    private:
        ImGuiContext* imgui_create_context(const Function<void(ImGuiContext*)>& callback);
        void imgui_destroy_context(ImGuiContext* context);

    public:
        virtual void initialize(const WindowConfig&);
        virtual Size1D width();
        virtual Window& width(const Size1D& width);
        virtual Size1D height();
        virtual Window& height(const Size1D& height);
        virtual Size2D size();
        virtual Window& size(const Size2D& size);
        virtual String title();
        virtual Window& title(const String& title);
        virtual Point2D position();
        virtual Window& position(const Point2D& position);
        virtual bool resizable();
        virtual Window& resizable(bool value);
        virtual Window& focus();
        virtual bool focused();
        virtual Window& show();
        virtual Window& hide();
        virtual bool is_visible();
        virtual bool is_iconify();
        virtual Window& iconify();
        virtual bool is_restored();
        virtual Window& restore();
        virtual Window& opacity(float value);
        virtual float opacity();
        virtual Window& icon(const Image& image);
        virtual Window& cursor(const Image& image, IntVector2D hotspot = {0, 0});
        virtual Window& attribute(const WindowAttribute& attrib, bool value);
        virtual bool attribute(const WindowAttribute& attrib);
        virtual Window& cursor_mode(const CursorMode& mode);
        virtual CursorMode cursor_mode();
        virtual bool support_orientation(WindowOrientation orientation);
        virtual Identifier id();
        virtual void* native_window();
        virtual Window& imgui_new_frame();


        RenderViewport* render_viewport() const;
        Window* parent_window() const;
        const Vector<Window*>& child_windows() const;

        ImGuiRenderer::Window* imgui_window();
        Window& imgui_initialize(const Function<void(ImGuiContext*)>& callback = {});
        Window& imgui_terminate();

        Identifier register_destroy_callback(const DestroyCallback& callback);
        Window& unregister_destroy_callback(Identifier id);

        Size2D cached_size() const;
        Window& update_cached_size();
        Window& create_client(const StringView& client_name);

        virtual ~Window();

        friend class WindowManager;
        friend class RenderViewport;
    };
}// namespace Engine
