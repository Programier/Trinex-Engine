#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/logger.hpp>
#include <Core/thread.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <imgui.h>

namespace Engine
{
    implement_class(Window, "Engine");
    implement_default_initialize_class(Window);

    class WindowRenderPass : public RenderPass
    {
    public:
        WindowRenderPass()
        {}


        WindowRenderPass& rhi_create()
        {
            _M_rhi_object.reset(engine_instance->rhi()->window_render_pass());
            return *this;
        }
    };

    Window::Window(WindowInterface* interface, bool vsync) : _M_interface(interface)
    {
        _M_render_viewport = Object::new_instance<RenderViewport>();
        _M_render_viewport->flag(Object::Flag::IsAvailableForGC, false);
        _M_render_viewport->window(this, vsync);
        _M_render_viewport->init_resource();
        engine_instance->thread(ThreadType::RenderThread)->wait_all();

        _M_rhi_object.reset(_M_render_viewport->render_target());
        render_pass = &Object::new_instance<WindowRenderPass>()->rhi_create();

        rhi_create();
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

    ImGuiRenderer::Window* Window::imgui_window()
    {
        return _M_imgui_window;
    }


    struct InitContext : public ExecutableObject {
        ImGuiContext* _M_ctx;
        RHI* _M_rhi;

        InitContext(RHI* rhi, ImGuiContext* ctx) : _M_ctx(ctx), _M_rhi(rhi)
        {}

        int_t execute() override
        {
            _M_rhi->imgui_init(_M_ctx);
            return sizeof(InitContext);
        }
    };


    struct TerminateContext : public ExecutableObject {
        ImGuiContext* _M_ctx;
        RHI* _M_rhi;

        TerminateContext(RHI* rhi, ImGuiContext* ctx) : _M_ctx(ctx), _M_rhi(rhi)
        {}

        int_t execute() override
        {
            _M_rhi->imgui_terminate(_M_ctx);
            return sizeof(InitContext);
        }
    };


    static ImGuiContext* imgui_create_context(WindowInterface* interface)
    {
        ImGuiContext* context = ImGui::CreateContext();
        Thread* render_thread = engine_instance->thread(ThreadType::RenderThread);
        RHI* rhi              = engine_instance->rhi();

        ImGui::SetCurrentContext(context);
        ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        interface->initialize_imgui();

        render_thread->insert_new_task<InitContext>(rhi, context);
        return context;
    }

    static void imgui_destroy_context(ImGuiContext* context, WindowInterface* interface)
    {
        Thread* render_thread = engine_instance->thread(ThreadType::RenderThread);
        RHI* rhi              = engine_instance->rhi();

        render_thread->insert_new_task<TerminateContext>(rhi, context);
        render_thread->wait_all();

        ImGui::SetCurrentContext(context);
        interface->terminate_imgui();

        ImGui::DestroyContext(context);
    }

    Window& Window::imgui_initialize(const Function<void(ImGuiContext*)>& callback)
    {
        if (!_M_imgui_window)
        {
            ImGuiContext* current_context = ImGui::GetCurrentContext();
            _M_imgui_window               = new ImGuiRenderer::Window(_M_interface, imgui_create_context(_M_interface));
            engine_instance->thread(ThreadType::RenderThread)->wait_all();

            if (callback)
            {
                callback(_M_imgui_window->context());
            }
            ImGui::SetCurrentContext(current_context);
        }

        return *this;
    }

    Window& Window::imgui_terminate()
    {
        if (_M_imgui_window)
        {
            ImGuiContext* current_context = ImGui::GetCurrentContext();
            _M_imgui_window->free_resources();

            imgui_destroy_context(_M_imgui_window->context(), _M_interface);
            ImGui::SetCurrentContext(current_context);

            delete _M_imgui_window;
            _M_imgui_window = nullptr;
            return *this;
        }

        return *this;
    }

    Window::~Window()
    {
        _M_destroy_callback.trigger();
        imgui_terminate();

        delete _M_render_viewport;
        engine_instance->thread(ThreadType::RenderThread)->wait_all();
        delete _M_interface;

        // The window cannot remove the render target because it is a viewport resource
        _M_rhi_object.release();
    }

    Identifier Window::register_destroy_callback(const DestroyCallback& callback)
    {
        return _M_destroy_callback.push(callback);
    }

    Window& Window::unregister_destroy_callback(Identifier id)
    {
        _M_destroy_callback.remove(id);
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

    WindowInterface* Window::interface() const
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

    int_t Window::create_message_box(const MessageBoxCreateInfo& info)
    {
        return _M_interface->create_message_box(info);
    }

    RenderViewport* Window::render_viewport() const
    {
        return _M_render_viewport;
    }

    Window* Window::parent_window() const
    {
        return _M_parent_window;
    }

    const Vector<Window*>& Window::child_windows() const
    {
        return _M_childs;
    }

    Identifier Window::window_id() const
    {
        return _M_interface->id();
    }
}// namespace Engine
