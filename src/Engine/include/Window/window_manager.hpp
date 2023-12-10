#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/singletone.hpp>
#include <Window/window_manager_interface.hpp>

namespace Engine
{
    class WindowManagerInterface;
    class Window;
    struct WindowConfig;
    struct MonitorInfo;

    class ENGINE_EXPORT WindowManager final : public Singletone<WindowManager, EmptyClass>
    {
    private:
        static WindowManager* _M_instance;
        WindowManagerInterface* _M_interface = nullptr;
        TreeMap<Identifier, Window*> _M_windows;
        Window* _M_main_window = nullptr;


        WindowManager();

    public:
        ~WindowManager();
        Window* create_window(const WindowConfig& config, Window* parent);
        WindowManager& destroy_window(Window* window);

        WindowManager& create_notify(const NotifyCreateInfo& info);
        String error() const;
        bool has_error() const;
        bool mouse_relative_mode() const;
        WindowManager& mouse_relative_mode(bool flag);
        WindowManager& update_monitor_info(MonitorInfo& info);
        WindowManager& add_event_callback(Identifier system_id, const EventCallback& callback);
        WindowManager& remove_all_callbacks(Identifier system_id);
        WindowManager& start_text_input();
        WindowManager& stop_text_input();
        WindowManager& pool_events();
        WindowManager& wait_for_events();
        Window* find(Identifier id) const;

        Size2D calculate_gbuffer_size() const;
        Window* main_window() const;
        const TreeMap<Identifier, Window*>& windows() const;

        friend class Window;
        friend class Singletone<WindowManager, EmptyClass>;
        friend class Object;
    };
}// namespace Engine
