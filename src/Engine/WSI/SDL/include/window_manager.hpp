#pragma once
#include <SDL.h>
#include <Window/window_manager_interface.hpp>

namespace Engine
{
    class SDL2_WindowManagerInterface : public WindowManagerInterface
    {
    private:
        SDL_Event m_event;
        Map<Sint32, SDL_GameController*> m_game_controllers;
        Map<Identifier, Vector<EventCallback>> m_event_callbacks;

    public:
        SDL2_WindowManagerInterface();

        WindowInterface* create_window(const WindowConfig* config) override;                                     // +
        WindowManagerInterface& destroy_window(WindowInterface* interface) override;                             // +
        WindowManagerInterface& create_notify(const NotifyCreateInfo& info) override;                            // +
        String error() const override;                                                                           // +
        bool has_error() const override;                                                                         // +
        bool mouse_relative_mode() const override;                                                               // +
        WindowManagerInterface& mouse_relative_mode(bool flag) override;                                         // +
        WindowManagerInterface& update_monitor_info(MonitorInfo& info) override;                                 // +
        WindowManagerInterface& add_event_callback(Identifier system_id, const EventCallback& callback) override;// +
        WindowManagerInterface& remove_all_callbacks(Identifier system_id) override;                             // +
        WindowManagerInterface& start_text_input() override;                                                     // +
        WindowManagerInterface& stop_text_input() override;                                                      // =
        WindowManagerInterface& pool_events_loop();
        WindowManagerInterface& pool_events() override;
        WindowManagerInterface& wait_for_events() override;

        bool show_splash_screen(const class Image& image, Size2D splash_size, const SplashConfig& config) override;
        WindowManagerInterface& update_splash_screen() override;
        WindowManagerInterface& update_splash_screen_text(SplashTextType type, const StringView& text) override;
        WindowManagerInterface& hide_splash_screen() override;

        void process_event();
        void process_imgui_event();
        void send_event(const Event& event);
        void process_mouse_button();
        ~SDL2_WindowManagerInterface() override;
    };
}// namespace Engine
