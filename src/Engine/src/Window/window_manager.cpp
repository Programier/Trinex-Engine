#include <Core/class.hpp>
#include <Core/constants.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/exception.hpp>
#include <Core/library.hpp>
#include <Core/render_thread_call.hpp>
#include <Core/string_functions.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>


namespace Engine
{

    WindowManager* WindowManager::_M_instance = nullptr;

    WindowManager::WindowManager()
    {
        String libname = Strings::format("WindowSystem{}", engine_config.window_system);
        Library library(libname);

        if (!library.has_lib())
        {
            throw EngineException("Cannot load window system library!");
        }

        WindowManagerInterface* (*loader)() =
                library.get<WindowManagerInterface*>(Constants::library_load_function_name);
        if (!loader)
        {
            throw EngineException("Cannot create window manager loader");
        }

        _M_interface = loader();

        if (_M_interface == nullptr)
        {
            throw EngineException("Cannot create window manager");
        }
    }

    WindowManager& WindowManager::destroy_window(Window* window)
    {
        if (window)
        {
            RHI* rhi = engine_instance->rhi();
            call_in_render_thread([rhi]() { rhi->wait_idle(); });
            engine_instance->thread(ThreadType::RenderThread)->wait_all();

            if (window == _M_main_window)
                _M_main_window = nullptr;
            _M_windows.erase(window->window_id());

            while (!window->_M_childs.empty())
            {
                destroy_window(window->_M_childs.back());
            }

            if (window->_M_parent_window)
            {
                auto& childs = window->_M_parent_window->_M_childs;
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
        while (!_M_windows.empty())
        {
            Window* window = _M_windows.begin()->second;
            destroy_window(window);
        }

        delete _M_interface;
    }


    Window* WindowManager::create_window(const WindowConfig& config, Window* parent)
    {
        WindowInterface* window_interface = _M_interface->create_window(&config);

        if (window_interface == nullptr)
            return nullptr;

        Window* window = new Window(window_interface, config.vsync);

        parent = parent ? parent : _M_main_window;
        if (parent)
        {
            parent->_M_childs.push_back(window);
            window->_M_parent_window = parent;
        }

        if (_M_windows.empty())
        {
            _M_main_window = window;
        }

        _M_windows[window->window_id()] = window;

        // Initialize client
        Class* client_class = Class::static_find_class(config.client);
        if (client_class)
        {
            Object* object         = client_class->create_object();
            ViewportClient* client = object->instance_cast<ViewportClient>();
            if (client)
            {
                window->render_viewport()->client(client);
            }
            else if (object)
            {
                delete object;
            }
        }

        return window;
    }


    WindowManager& WindowManager::create_notify(const NotifyCreateInfo& info)
    {
        _M_interface->create_notify(info);
        return *this;
    }

    String WindowManager::error() const
    {
        return _M_interface->error();
    }

    bool WindowManager::has_error() const
    {
        return _M_interface->has_error();
    }

    bool WindowManager::mouse_relative_mode() const
    {
        return _M_interface->mouse_relative_mode();
    }

    WindowManager& WindowManager::mouse_relative_mode(bool flag)
    {
        _M_interface->mouse_relative_mode(flag);
        return *this;
    }

    WindowManager& WindowManager::update_monitor_info(MonitorInfo& info)
    {
        _M_interface->update_monitor_info(info);
        return *this;
    }

    WindowManager& WindowManager::add_event_callback(Identifier system_id, const EventCallback& callback)
    {
        _M_interface->add_event_callback(system_id, callback);
        return *this;
    }

    WindowManager& WindowManager::remove_all_callbacks(Identifier system_id)
    {
        _M_interface->remove_all_callbacks(system_id);
        return *this;
    }

    WindowManager& WindowManager::start_text_input()
    {
        _M_interface->start_text_input();
        return *this;
    }

    WindowManager& WindowManager::stop_text_input()
    {
        _M_interface->stop_text_input();
        return *this;
    }

    WindowManager& WindowManager::pool_events()
    {
        _M_interface->pool_events();
        return *this;
    }

    WindowManager& WindowManager::wait_for_events()
    {
        _M_interface->wait_for_events();
        return *this;
    }

    Window* WindowManager::find(Identifier id) const
    {
        auto it = _M_windows.find(id);
        if (it == _M_windows.end())
            return nullptr;
        return it->second;
    }

    Size2D WindowManager::calculate_gbuffer_size() const
    {
        Size2D size = {0, 0};

        for (auto& [id, window] : _M_windows)
        {
            size = glm::max(size, window->cached_size());
        }

        return size;
    }

    Window* WindowManager::main_window() const
    {
        return _M_main_window;
    }

    const TreeMap<Identifier, Window*>& WindowManager::windows() const
    {
        return _M_windows;
    }
}// namespace Engine
