#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>


namespace Engine
{
    implement_class(Window, "Engine");
    implement_default_initialize_class(Window);

    class WindowRenderPass : public RenderPass
    {
    public:
        WindowRenderPass()
        {
            _M_default_render_passes[RenderPass::Type::Window] = this;
        }

        WindowRenderPass& rhi_create()
        {
            _M_can_delete      = false;
            _M_rhi_render_pass = engine_instance->rhi()->window_render_pass();
            return *this;
        }
    };

    Window::Window(WindowInterface* interface) : _M_interface(interface)
    {
        _M_rhi_render_target = EngineInstance::instance()->rhi()->window_render_target();
        render_pass          = &Object::new_instance<WindowRenderPass>()->rhi_create();


        flag(Object::IsAvailableForGC, false);
        update_cached_size();

        _M_viewport.pos       = {0, 0};
        _M_viewport.size      = size();
        _M_viewport.min_depth = 0.0f;
        _M_viewport.max_depth = 1.0f;

        _M_scissor.pos  = {0, 0};
        _M_scissor.size = _M_viewport.size;


        viewport(_M_viewport);
        scissor(_M_scissor);
    }

    void Window::close()
    {
        _M_interface->close();
    }

    bool Window::is_open()
    {
        return _M_interface->is_open();
    }

    Size1D Window::width()
    {
        return _M_interface->width();
    }

    Window& Window::width(const Size1D& width)
    {
        _M_interface->width(width);
        return *this;
    }

    Size1D Window::height()
    {
        return _M_interface->height();
    }

    Window& Window::height(const Size1D& height)
    {
        _M_interface->height(height);
        return *this;
    }

    Size2D Window::size()
    {
        return _M_interface->size();
    }

    Window& Window::size(const Size2D& size)
    {
        _M_interface->size(size);
        return *this;
    }

    String Window::title()
    {
        return _M_interface->title();
    }

    Window& Window::title(const String& title)
    {
        _M_interface->title(title);
        return *this;
    }

    Point2D Window::position()
    {
        return _M_interface->position();
    }

    Window& Window::position(const Point2D& position)
    {
        _M_interface->position(position);
        return *this;
    }

    bool Window::resizable()
    {
        return _M_interface->resizable();
    }

    Window& Window::resizable(bool value)
    {
        _M_interface->resizable(value);
        return *this;
    }

    Window& Window::focus()
    {
        _M_interface->focus();
        return *this;
    }

    bool Window::focused()
    {
        return _M_interface->focused();
    }

    Window& Window::show()
    {
        _M_interface->show();
        return *this;
    }

    Window& Window::hide()
    {
        _M_interface->hide();
        return *this;
    }

    bool Window::is_visible()
    {
        return _M_interface->is_visible();
    }

    bool Window::is_iconify()
    {
        return _M_interface->is_iconify();
    }

    Window& Window::iconify()
    {
        _M_interface->iconify();
        return *this;
    }

    bool Window::is_restored()
    {
        return _M_interface->is_restored();
    }

    Window& Window::restore()
    {
        _M_interface->restore();
        return *this;
    }

    Window& Window::opacity(float value)
    {
        _M_interface->opacity(value);
        return *this;
    }

    float Window::opacity()
    {
        return _M_interface->opacity();
    }

    Window& Window::size_limits(const SizeLimits2D& limits)
    {
        _M_interface->size_limits(limits);
        return *this;
    }

    SizeLimits2D Window::size_limits()
    {
        return _M_interface->size_limits();
    }

    Window& Window::attribute(const WindowAttribute& attrib, bool value)
    {
        _M_interface->attribute(attrib, value);
        return *this;
    }

    bool Window::attribute(const WindowAttribute& attrib)
    {
        return _M_interface->attribute(attrib);
    }

    Window& Window::cursor_mode(const CursorMode& mode)
    {
        _M_interface->cursor_mode(mode);
        return *this;
    }

    CursorMode Window::cursor_mode()
    {
        return _M_interface->cursor_mode();
    }

    bool Window::support_orientation(WindowOrientation orientation)
    {
        return _M_interface->support_orientation(orientation);
    }

    Window& Window::start_text_input()
    {
        _M_interface->start_text_input();
        return *this;
    }

    Window& Window::stop_text_input()
    {
        _M_interface->stop_text_input();
        return *this;
    }

    Window& Window::pool_events()
    {
        _M_interface->pool_events();
        return *this;
    }

    Window& Window::wait_for_events()
    {
        _M_interface->wait_for_events();
        return *this;
    }

    void Window::initialize_imgui()
    {
        _M_interface->initialize_imgui();
    }

    void Window::terminate_imgui()
    {
        _M_interface->terminate_imgui();
    }

    void Window::new_imgui_frame()
    {
        _M_interface->new_imgui_frame();
    }

    Window::~Window()
    {
        _M_rhi_render_target = nullptr;// Window render target must be destroyed by API
        delete _M_interface;
    }

    Window* Window::instance()
    {
        return engine_instance->window();
    }

    Window& Window::swap_buffers()
    {
        engine_instance->rhi()->swap_buffer();
        return *this;
    }

    Window& Window::update_cached_size()
    {
        _M_cached_size = size();
        return *this;
    }

    const Size2D& Window::cached_size() const
    {
        return _M_cached_size;
    }

    void* Window::interface() const
    {
        return _M_interface;
    }

    Window& Window::icon(const Image& image)
    {
        _M_interface->window_icon(image);
        return *this;
    }

    Window& Window::cursor(const Image& image, IntVector2D hotspot)
    {
        _M_interface->cursor(image, hotspot);
        return *this;
    }

    Window& Window::update_monitor_info(MonitorInfo& info)
    {
        _M_interface->update_monitor_info(info);
        return *this;
    }

    int_t Window::create_message_box(const MessageBoxCreateInfo& info)
    {
        return _M_interface->create_message_box(info);
    }

    Window& Window::create_notify(const NotifyCreateInfo& info)
    {
        _M_interface->create_notify(info);
        return *this;
    }
}// namespace Engine
