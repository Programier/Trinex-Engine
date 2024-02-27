#include "logo.hpp"
#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/exception.hpp>
#include <Core/library.hpp>
#include <Core/render_thread.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Image/image.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <Window/monitor.hpp>


namespace Engine
{

    static const Image& load_image_icon()
    {
        static Image image;

        if (image.empty())
        {
            image.load_from_memory(logo_png, logo_png_len);
        }

        return image;
    }

    WindowManager* WindowManager::m_instance = nullptr;

    WindowManager::WindowManager()
    {
        String libname = Strings::format("WindowSystem{}", engine_config.window_system);
        Library library(libname);

        if (!library.has_lib())
        {
            throw EngineException("Cannot load window system library!");
        }

        WindowManagerInterface* (*loader)() = library.get<WindowManagerInterface*>(Constants::library_load_function_name);
        if (!loader)
        {
            throw EngineException("Cannot create window manager loader");
        }

        m_interface = loader();

        if (m_interface == nullptr)
        {
            throw EngineException("Cannot create window manager");
        }
    }

    WindowManager& WindowManager::destroy_window(Window* window)
    {
        if (window)
        {
            RHI* rhi = engine_instance->rhi();
            if (rhi)
            {
                call_in_render_thread([rhi]() { rhi->wait_idle(); });
                engine_instance->thread(ThreadType::RenderThread)->wait_all();
            }

            if (window == m_main_window)
                m_main_window = nullptr;
            m_windows.erase(window->window_id());

            while (!window->m_childs.empty())
            {
                destroy_window(window->m_childs.back());
            }

            if (window->m_parent_window)
            {
                auto& childs = window->m_parent_window->m_childs;
                for (size_t i = 0, count = childs.size(); i < count; i++)
                {
                    if (childs[i] == window)
                    {
                        childs.erase(childs.begin() + i);
                        break;
                    }
                }
            }

            delete window;
        }
        return *this;
    }

    WindowManager::~WindowManager()
    {
        while (!m_windows.empty())
        {
            Window* window = m_windows.begin()->second;
            destroy_window(window);
        }

        delete m_interface;
    }


    Window* WindowManager::create_window(const WindowConfig& config, Window* parent, WindowInterface* window_interface)
    {
        if (window_interface == nullptr)
            window_interface = m_interface->create_window(&config);

        if (window_interface == nullptr)
            return nullptr;

        Window* window = new Window(window_interface, config.vsync);

        parent = parent ? parent : m_main_window;
        if (parent)
        {
            parent->m_childs.push_back(window);
            window->m_parent_window = parent;
        }

        if (m_windows.empty())
        {
            m_main_window = window;
            Monitor::update();
        }

        m_windows[window->window_id()] = window;

        // Initialize client
        create_client(window, config.client);

        window->icon(load_image_icon());
        return window;
    }


    WindowManager& WindowManager::create_notify(const NotifyCreateInfo& info)
    {
        m_interface->create_notify(info);
        return *this;
    }

    String WindowManager::error() const
    {
        return m_interface->error();
    }

    bool WindowManager::has_error() const
    {
        return m_interface->has_error();
    }

    bool WindowManager::mouse_relative_mode() const
    {
        return m_interface->mouse_relative_mode();
    }

    WindowManager& WindowManager::mouse_relative_mode(bool flag)
    {
        m_interface->mouse_relative_mode(flag);
        return *this;
    }

    WindowManager& WindowManager::update_monitor_info(MonitorInfo& info)
    {
        m_interface->update_monitor_info(info);
        return *this;
    }

    WindowManager& WindowManager::add_event_callback(Identifier system_id, const EventCallback& callback)
    {
        m_interface->add_event_callback(system_id, callback);
        return *this;
    }

    WindowManager& WindowManager::remove_all_callbacks(Identifier system_id)
    {
        m_interface->remove_all_callbacks(system_id);
        return *this;
    }

    WindowManager& WindowManager::start_text_input()
    {
        m_interface->start_text_input();
        return *this;
    }

    WindowManager& WindowManager::stop_text_input()
    {
        m_interface->stop_text_input();
        return *this;
    }

    WindowManager& WindowManager::pool_events()
    {
        m_interface->pool_events();
        return *this;
    }

    WindowManager& WindowManager::wait_for_events()
    {
        m_interface->wait_for_events();
        return *this;
    }

    Window* WindowManager::find(Identifier id) const
    {
        auto it = m_windows.find(id);
        if (it == m_windows.end())
            return nullptr;
        return it->second;
    }

    WindowManager& WindowManager::create_client(Window* window, const StringView& client_name)
    {
        ViewportClient* client = ViewportClient::create(client_name);
        if (client)
        {
            window->render_viewport()->client(client);
        }
        return *this;
    }

    Size2D WindowManager::calculate_gbuffer_size() const
    {
        Size2D size = {0, 0};

        for (auto& [id, window] : m_windows)
        {
            size = glm::max(size, window->cached_size());
        }

        return size;
    }

    Window* WindowManager::main_window() const
    {
        return m_main_window;
    }

    const TreeMap<Identifier, Window*>& WindowManager::windows() const
    {
        return m_windows;
    }
}// namespace Engine
