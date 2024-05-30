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
        static WindowManager* m_instance;
        WindowManagerInterface* m_interface = nullptr;
        TreeMap<Identifier, Window*> m_windows;
        Window* m_main_window = nullptr;

        WindowManager();

    public:
        ~WindowManager();
        Window* create_window(const WindowConfig& config, Window* parent = nullptr, WindowInterface* interface = nullptr);
        WindowManager& destroy_window(Window* window);

        bool mouse_relative_mode() const;
        WindowManager& mouse_relative_mode(bool flag);
        WindowManager& update_monitor_info(MonitorInfo& info);
        WindowManager& pool_events();
        WindowManager& wait_for_events();
        Window* find(Identifier id) const;
        WindowManager& create_client(Window* window, const StringView& client_name);

        Size2D calculate_gbuffer_size() const;
        Window* main_window() const;
        const TreeMap<Identifier, Window*>& windows() const;
        friend class Window;
        friend class Singletone<WindowManager, EmptyClass>;
    };
}// namespace Engine
