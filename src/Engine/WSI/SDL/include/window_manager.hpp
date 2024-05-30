#pragma once
#include <SDL2/SDL.h>
#include <Window/window_manager_interface.hpp>

namespace Engine
{
    class SDL2_WindowManagerInterface : public WindowManagerInterface
    {
    private:
        SDL_Event m_event;
        Map<Sint32, SDL_GameController*> m_game_controllers;

    public:
        SDL2_WindowManagerInterface();

        WindowInterface* create_window(const WindowConfig* config) override;        // +
        WindowManagerInterface& destroy_window(WindowInterface* interface) override;// +
        bool mouse_relative_mode() const override;                                  // +
        WindowManagerInterface& mouse_relative_mode(bool flag) override;            // +
        WindowManagerInterface& update_monitor_info(MonitorInfo& info) override;    // +
        WindowManagerInterface& pool_events_loop();
        WindowManagerInterface& pool_events() override;
        WindowManagerInterface& wait_for_events() override;

        void process_event();
        void process_window_event();
        void process_imgui_event();
        void send_event(const Event& event);
        void process_mouse_button();
        ~SDL2_WindowManagerInterface() override;
    };
}// namespace Engine
