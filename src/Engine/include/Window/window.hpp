#pragma once
#include <Graphics/render_target_base.hpp>
#include <Window/window_interface.hpp>

namespace Engine
{
    class ENGINE_EXPORT Window : public RenderTargetBase
    {
        declare_class(Window, RenderTargetBase);

    private:
        WindowInterface* _M_interface            = nullptr;
        class RenderViewport* _M_render_viewport = nullptr;
        Size2D _M_cached_size;

        Window* _M_parent_window = nullptr;
        Vector<Window*> _M_childs;

    public:
        Size1D width();
        Window& width(const Size1D& width);
        Size1D height();
        Window& height(const Size1D& height);
        Size2D size();
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

        Identifier window_id() const;

        void initialize_imgui(ImGuiContext* ctx);
        void terminate_imgui();
        void new_imgui_frame();

    private:
        Window(WindowInterface* interface = nullptr, bool vsync = true);
        ~Window();

        friend class Object;
        friend class WindowManager;
    };
}// namespace Engine
