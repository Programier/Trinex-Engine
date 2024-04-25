#pragma once
#include <Core/callback.hpp>
#include <Graphics/render_target_base.hpp>
#include <Window/window_interface.hpp>

struct ImGuiContext;
struct ImDrawData;

namespace Engine
{
    namespace ImGuiRenderer
    {
        class Window;
    }

    class ENGINE_EXPORT Window : public RenderTargetBase
    {
        declare_class(Window, RenderTargetBase);

    public:
        using DestroyCallback = CallBack<void()>;

    private:
        WindowInterface* m_interface            = nullptr;
        Pointer<class RenderViewport> m_render_viewport;
        Size2D m_cached_size;
        Window* m_parent_window = nullptr;
        Vector<Window*> m_childs;
        ImGuiRenderer::Window* m_imgui_window = nullptr;
        CallBacks<void()> m_destroy_callback;

    public:
        Size1D width();
        Window& width(const Size1D& width);
        Size1D height();
        Window& height(const Size1D& height);
        Size2D size();
        Size2D render_target_size() const override;
        Window& size(const Size2D& size);
        String title();
        Window& title(const String& title);
        Point2D position();
        Window& position(const Point2D& position);
        bool resizable();
        Window& resizable(bool value);
        Window& focus();
        bool focused();
        Window& show();
        Window& hide();
        bool is_visible();
        bool is_iconify();
        Window& iconify();
        bool is_restored();
        Window& restore();
        Window& opacity(float value);
        float opacity();
        Window& size_limits(const SizeLimits2D& limits);
        SizeLimits2D size_limits();
        Window& attribute(const WindowAttribute& attrib, bool value);
        bool attribute(const WindowAttribute& attrib);
        Window& cursor_mode(const CursorMode& mode);
        CursorMode cursor_mode();
        bool support_orientation(WindowOrientation orientation);
        Window& update_cached_size();
        const Size2D& cached_size() const;
        WindowInterface* interface() const;
        Window& icon(const Image& image);
        Window& cursor(const Image& image, IntVector2D hotspot = {0, 0});
        int_t create_message_box(const MessageBoxCreateInfo& info);
        RenderViewport* render_viewport() const;
        Window* parent_window() const;
        const Vector<Window*>& child_windows() const;
        bool is_engine_resource() const override;

        Identifier window_id() const;

        ImGuiRenderer::Window* imgui_window();
        Window& imgui_initialize(const Function<void(ImGuiContext*)>& callback = {});
        Window& imgui_terminate();

        Identifier register_destroy_callback(const DestroyCallback& callback);
        Window& unregister_destroy_callback(Identifier id);

    private:
        Window(WindowInterface* interface = nullptr, bool vsync = true);
        ~Window();

        friend class Object;
        friend class WindowManager;
    };
}// namespace Engine
