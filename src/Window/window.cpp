#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/garbage_collector.hpp>
#include <Core/logger.hpp>
#include <Core/threading.hpp>
#include <Graphics/imgui.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <imgui.h>

namespace Engine
{

    Size1D Window::width()
    {
        return 0.f;
    }

    Window& Window::width(const Size1D& width)
    {
        return *this;
    }

    Size1D Window::height()
    {
        return 0.f;
    }

    Window& Window::height(const Size1D& height)
    {
        return *this;
    }

    Size2D Window::size()
    {
        return {width(), height()};
    }

    Window& Window::size(const Size2D& size)
    {
        return *this;
    }

    String Window::title()
    {
        return "Trinex Engine Window";
    }

    Window& Window::title(const String& title)
    {
        return *this;
    }

    Point2D Window::position()
    {
        return {0.f, 0.f};
    }

    Window& Window::position(const Point2D& position)
    {
        return *this;
    }

    bool Window::resizable()
    {
        return false;
    }

    Window& Window::resizable(bool value)
    {
        return *this;
    }

    Window& Window::focus()
    {
        return *this;
    }

    bool Window::focused()
    {
        return false;
    }

    Window& Window::show()
    {
        return *this;
    }

    Window& Window::hide()
    {
        return *this;
    }

    bool Window::is_visible()
    {
        return false;
    }

    bool Window::is_iconify()
    {
        return false;
    }

    Window& Window::iconify()
    {
        return *this;
    }

    bool Window::is_restored()
    {
        return false;
    }

    Window& Window::restore()
    {
        return *this;
    }

    Window& Window::opacity(float value)
    {
        return *this;
    }

    float Window::opacity()
    {
        return 1.f;
    }

    Window& Window::icon(const Image& image)
    {
        return *this;
    }

    Window& Window::cursor(const Image& image, IntVector2D hotspot)
    {
        return *this;
    }

    Window& Window::attribute(const WindowAttribute& attrib, bool value)
    {
        return *this;
    }

    bool Window::attribute(const WindowAttribute& attrib)
    {
        return false;
    }

    Window& Window::cursor_mode(const CursorMode& mode)
    {
        return *this;
    }

    CursorMode Window::cursor_mode()
    {
        return CursorMode::Normal;
    }

    bool Window::support_orientation(WindowOrientation orientation)
    {
        return false;
    }

    Identifier Window::id()
    {
        return reinterpret_cast<Identifier>(this);
    }

    void* Window::native_window()
    {
        return nullptr;
    }

    Window& Window::imgui_initialize_internal()
    {
        return *this;
    }

    Window& Window::imgui_terminate_internal()
    {
        return *this;
    }

    Window& Window::imgui_new_frame()
    {
        return *this;
    }

    void Window::initialize(const WindowConfig& config)
    {
        m_render_viewport = Object::new_instance<RenderViewport>();
        m_render_viewport->window(this, config.vsync);
        m_render_viewport->init_resource(true);

        update_cached_size();

        if (!InitializeController().is_triggered())
        {
            // Default resources is not loaded now, so, using deferred initialization
            InitializeController().push([this, client = config.client]() { create_client(client); });
        }
        else
        {
            create_client(config.client);
        }
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

    ImGuiRenderer::Window* Window::imgui_window()
    {
        return m_imgui_window.ptr();
    }

    namespace ImGuiBackend
    {
        extern bool imgui_trinex_init(ImGuiContext* ctx);
        extern void imgui_trinex_shutdown(ImGuiContext* ctx);
    }// namespace ImGuiBackend

    struct InitContext : public ExecutableObject {
        ImGuiContext* m_ctx;

        InitContext(ImGuiContext* ctx) : m_ctx(ctx)
        {}

        int_t execute() override
        {
            ImGuiBackend::imgui_trinex_init(m_ctx);
            return sizeof(InitContext);
        }
    };


    struct TerminateContext : public ExecutableObject {
        ImGuiContext* m_ctx;

        TerminateContext(ImGuiContext* ctx) : m_ctx(ctx)
        {}

        int_t execute() override
        {
            ImGuiBackend::imgui_trinex_shutdown(m_ctx);
            return sizeof(TerminateContext);
        }
    };


    ImGuiContext* Window::imgui_create_context(const Function<void(ImGuiContext*)>& callback)
    {
        ImGuiContext* context = ImGui::CreateContext();

        Thread* rt = render_thread();
        ImGui::SetCurrentContext(context);

        if (callback)
        {
            callback(context);
        }
#if !PLATFORM_ANDROID
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        imgui_initialize_internal();

        rt->insert_new_task<InitContext>(context);
        rt->wait_all();
        return context;
    }

    void Window::imgui_destroy_context(ImGuiContext* context)
    {
        Thread* rt = render_thread();
        rt->insert_new_task<TerminateContext>(context);
        rt->wait_all();

        ImGui::SetCurrentContext(context);
        imgui_terminate_internal();

        ImGui::DestroyContext(context);
    }

    Window& Window::imgui_initialize(const Function<void(ImGuiContext*)>& callback)
    {
        if (!m_imgui_window)
        {
            ImGuiContext* current_context = ImGui::GetCurrentContext();
            m_imgui_window                = Object::new_instance<ImGuiRenderer::Window>(this, imgui_create_context(callback));
            ImGui::SetCurrentContext(current_context);
        }

        return *this;
    }

    Window& Window::imgui_terminate()
    {
        if (m_imgui_window)
        {
            ImGuiRenderer::Window* current_window = ImGuiRenderer::Window::current();
            if (m_imgui_window.ptr() == current_window)
                current_window = nullptr;

            m_imgui_window->free_resources();

            imgui_destroy_context(m_imgui_window->context());
            m_imgui_window->m_window = nullptr;
            m_imgui_window           = nullptr;

            ImGuiRenderer::Window::make_current(current_window);
            return *this;
        }

        return *this;
    }

    Window::~Window()
    {
        m_destroy_callback.trigger();
        imgui_terminate();

        if (m_render_viewport)
        {
            RenderViewport* viewport = m_render_viewport;
            m_render_viewport        = nullptr;
            GarbageCollector::destroy(viewport);
        }
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

    Size2D Window::cached_size() const
    {
        return m_cached_size;
    }

    Window& Window::update_cached_size()
    {
        m_cached_size = size();
        return *this;
    }

    Window& Window::create_client(const StringView& client_name)
    {
        ViewportClient* client = ViewportClient::create(client_name);
        if (client)
        {
            render_viewport()->client(client);
        }
        return *this;
    }
}// namespace Engine
