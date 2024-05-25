#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/logger.hpp>
#include <Core/threading.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_pass.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>
#include <Window/window_interface.hpp>
#include <imgui.h>

namespace Engine
{
    implement_class(Window, Engine, 0);
    implement_default_initialize_class(Window);

    class WindowRenderPass : public RenderPass
    {
    public:
        WindowRenderPass()
        {}

        WindowRenderPass& rhi_create() override
        {
            m_rhi_object.reset(rhi->window_render_pass(this));
            return *this;
        }

        RenderPassType type() const override
        {
            return RenderPassType::Window;
        }

        bool is_engine_resource() const override
        {
            return true;
        }
    };

    RenderPass* RenderPass::load_window_render_pass()
    {
        RenderPass* render_pass = Object::new_instance<WindowRenderPass>();
        render_pass->init_resource(true);
        return render_pass;
    }

    Window::Window(WindowInterface* interface, bool vsync) : m_interface(interface)
    {
        flags(Object::IsAvailableForGC, false);

        m_render_viewport = Object::new_instance<RenderViewport>();
        m_render_viewport->flags(Object::Flag::IsAvailableForGC, false);
        m_render_viewport->window(this, vsync);
        m_render_viewport->init_resource(true);

        m_rhi_object.reset(m_render_viewport->rhi_render_target());
        render_thread()->wait_all();

        render_pass = RenderPass::load_render_pass(RenderPassType::Window);

        update_cached_size();

        m_viewport.pos       = {0, 0};
        m_viewport.size      = size();
        m_viewport.min_depth = 0.0f;
        m_viewport.max_depth = 1.0f;

        m_scissor.pos  = {0, 0};
        m_scissor.size = m_viewport.size;


        // Need update viewport and scissor on rhi side
        viewport(m_viewport);
        scissor(m_scissor);


        init_resource();
    }


    Size1D Window::width()
    {
        return m_interface->width();
    }

    Window& Window::width(const Size1D& width)
    {
        m_interface->width(width);
        return *this;
    }

    Size1D Window::height()
    {
        return m_interface->height();
    }

    Window& Window::height(const Size1D& height)
    {
        m_interface->height(height);
        return *this;
    }

    Size2D Window::size()
    {
        return m_interface->size();
    }

    Size2D Window::render_target_size() const
    {
        return m_interface->size();
    }

    Window& Window::size(const Size2D& size)
    {
        m_interface->size(size);
        return *this;
    }

    String Window::title()
    {
        return m_interface->title();
    }

    Window& Window::title(const String& title)
    {
        m_interface->title(title);
        return *this;
    }

    Point2D Window::position()
    {
        return m_interface->position();
    }

    Window& Window::position(const Point2D& position)
    {
        m_interface->position(position);
        return *this;
    }

    bool Window::resizable()
    {
        return m_interface->resizable();
    }

    Window& Window::resizable(bool value)
    {
        m_interface->resizable(value);
        return *this;
    }

    Window& Window::focus()
    {
        m_interface->focus();
        return *this;
    }

    bool Window::focused()
    {
        return m_interface->focused();
    }

    Window& Window::show()
    {
        m_interface->show();
        return *this;
    }

    Window& Window::hide()
    {
        m_interface->hide();
        return *this;
    }

    bool Window::is_visible()
    {
        return m_interface->is_visible();
    }

    bool Window::is_iconify()
    {
        return m_interface->is_iconify();
    }

    Window& Window::iconify()
    {
        m_interface->iconify();
        return *this;
    }

    bool Window::is_restored()
    {
        return m_interface->is_restored();
    }

    Window& Window::restore()
    {
        m_interface->restore();
        return *this;
    }

    Window& Window::opacity(float value)
    {
        m_interface->opacity(value);
        return *this;
    }

    float Window::opacity()
    {
        return m_interface->opacity();
    }

    Window& Window::size_limits(const SizeLimits2D& limits)
    {
        m_interface->size_limits(limits);
        return *this;
    }

    SizeLimits2D Window::size_limits()
    {
        return m_interface->size_limits();
    }

    Window& Window::attribute(const WindowAttribute& attrib, bool value)
    {
        m_interface->attribute(attrib, value);
        return *this;
    }

    bool Window::attribute(const WindowAttribute& attrib)
    {
        return m_interface->attribute(attrib);
    }

    Window& Window::cursor_mode(const CursorMode& mode)
    {
        m_interface->cursor_mode(mode);
        return *this;
    }

    CursorMode Window::cursor_mode()
    {
        return m_interface->cursor_mode();
    }

    bool Window::support_orientation(WindowOrientation orientation)
    {
        return m_interface->support_orientation(orientation);
    }

    ImGuiRenderer::Window* Window::imgui_window()
    {
        return m_imgui_window;
    }


    struct InitContext : public ExecutableObject {
        ImGuiContext* m_ctx;
        RHI* m_rhi;

        InitContext(RHI* rhi, ImGuiContext* ctx) : m_ctx(ctx), m_rhi(rhi)
        {}

        int_t execute() override
        {
            m_rhi->imgui_init(m_ctx);
            return sizeof(InitContext);
        }
    };


    struct TerminateContext : public ExecutableObject {
        ImGuiContext* m_ctx;
        RHI* m_rhi;

        TerminateContext(RHI* rhi, ImGuiContext* ctx) : m_ctx(ctx), m_rhi(rhi)
        {}

        int_t execute() override
        {
            m_rhi->imgui_terminate(m_ctx);
            return sizeof(TerminateContext);
        }
    };


    static ImGuiContext* imgui_create_context(WindowInterface* interface, const Function<void(ImGuiContext*)>& callback)
    {
        ImGuiContext* context = ImGui::CreateContext();

        Thread* rt = render_thread();
        ImGui::SetCurrentContext(context);

        if (callback)
        {
            callback(context);
        }

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        interface->initialize_imgui();

        rt->insert_new_task<InitContext>(rhi, context);
        rt->wait_all();
        return context;
    }

    static void imgui_destroy_context(ImGuiContext* context, WindowInterface* interface)
    {
        Thread* rt = render_thread();
        rt->insert_new_task<TerminateContext>(rhi, context);
        rt->wait_all();

        ImGui::SetCurrentContext(context);
        interface->terminate_imgui();

        ImGui::DestroyContext(context);
    }

    Window& Window::imgui_initialize(const Function<void(ImGuiContext*)>& callback)
    {
        if (!m_imgui_window)
        {
            ImGuiContext* current_context = ImGui::GetCurrentContext();
            m_imgui_window                = new ImGuiRenderer::Window(this, imgui_create_context(m_interface, callback));
            ImGui::SetCurrentContext(current_context);
        }

        return *this;
    }

    Window& Window::imgui_terminate()
    {
        if (m_imgui_window)
        {
            ImGuiRenderer::Window* current_window = ImGuiRenderer::Window::current();
            if (m_imgui_window == current_window)
                current_window = nullptr;

            m_imgui_window->free_resources();

            imgui_destroy_context(m_imgui_window->context(), m_interface);

            delete m_imgui_window;
            m_imgui_window = nullptr;

            ImGuiRenderer::Window::make_current(current_window);
            return *this;
        }

        return *this;
    }

    Window::~Window()
    {
        m_destroy_callback.trigger();
        imgui_terminate();

        delete m_render_viewport;
        render_thread()->wait_all();
        delete m_interface;

        // The window cannot remove the render target because it is a viewport resource
        m_rhi_object.release();
    }

    Identifier Window::register_destroy_callback(const DestroyCallback& callback)
    {
        return m_destroy_callback.push(callback);
    }

    Window& Window::unregister_destroy_callback(Identifier id)
    {
        m_destroy_callback.remove(id);
        return *this;
    }

    Window& Window::update_cached_size()
    {
        m_cached_size = size();
        return *this;
    }

    const Size2D& Window::cached_size() const
    {
        return m_cached_size;
    }

    WindowInterface* Window::interface() const
    {
        return m_interface;
    }

    Window& Window::icon(const Image& image)
    {
        m_interface->window_icon(image);
        return *this;
    }

    Window& Window::cursor(const Image& image, IntVector2D hotspot)
    {
        m_interface->cursor(image, hotspot);
        return *this;
    }

    int_t Window::create_message_box(const MessageBoxCreateInfo& info)
    {
        return m_interface->create_message_box(info);
    }

    RenderViewport* Window::render_viewport() const
    {
        return m_render_viewport;
    }

    Window* Window::parent_window() const
    {
        return m_parent_window;
    }

    const Vector<Window*>& Window::child_windows() const
    {
        return m_childs;
    }

    bool Window::is_engine_resource() const
    {
        return true;
    }

    Identifier Window::window_id() const
    {
        return m_interface->id();
    }
}// namespace Engine
