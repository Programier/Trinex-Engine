#pragma once

#include <imgui.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <Window/window_interface.hpp>

namespace Engine
{
    struct WindowSDL : public WindowInterface {
        Set<void (*)(SDL_Event*)> on_event;
        Map<Identifier, Vector<EventCallback>> event_callbacks;

        Buffer _M_icon_buffer;
        Buffer _M_cursor_icon_buffer;

        SDL_Window* window          = nullptr;
        SDL_Surface* _M_icon        = nullptr;
        SDL_Surface* _M_cursor_icon = nullptr;
        SDL_Cursor* _M_cursor       = nullptr;

        SDL_GLContext gl_context;
        ImGuiContext* imgui_context = nullptr;
        SDL_WindowFlags api;
        CursorMode c_mode;
        VkSurfaceKHR vulkan_surface;
        SDL_Event event;


        void process_mouse_button();

        void init(const WindowConfig& info) override;
        void close() override;
        bool is_open() override;
        Size1D width() override;
        WindowInterface& width(const Size1D& width) override;
        Size1D height() override;
        WindowInterface& height(const Size1D& height) override;
        Size2D size() override;
        WindowInterface& size(const Size2D& size) override;
        String title() override;
        WindowInterface& title(const String& title) override;
        Point2D position() override;
        WindowInterface& position(const Point2D& position) override;
        bool rezisable() override;
        WindowInterface& rezisable(bool value) override;
        WindowInterface& focus() override;
        bool focused() override;
        WindowInterface& show() override;
        WindowInterface& hide() override;
        bool is_visible() override;
        bool is_iconify() override;
        WindowInterface& iconify() override;
        bool is_restored() override;
        WindowInterface& restore() override;
        WindowInterface& opacity(float value) override;
        float opacity() override;
        WindowInterface& size_limits(const SizeLimits2D& limits) override;
        SizeLimits2D size_limits() override;
        WindowInterface& window_icon(const Image& image) override;
        WindowInterface& cursor(const Image& image, IntVector2D hotspot) override;
        WindowInterface& attribute(const WindowAttribute& attrib, bool value) override;
        bool attribute(const WindowAttribute& attrib) override;
        WindowInterface& cursor_mode(const CursorMode& mode) override;
        CursorMode cursor_mode() override;
        WindowInterface& support_orientation(const Vector<WindowOrientation>& orientation) override;
        WindowInterface& start_text_input() override;
        WindowInterface& stop_text_input() override;
        WindowInterface& pool_events() override;
        WindowInterface& wait_for_events() override;
        void* create_surface(const char* any_text, ...) override;
        WindowInterface& swap_buffers() override;
        Vector<const char*> required_extensions() override;
        WindowInterface& add_event_callback(Identifier system_id, const EventCallback& callback) override;
        WindowInterface& remove_all_callbacks(Identifier system_id) override;
        bool mouse_relative_mode() const override;
        WindowInterface& mouse_relative_mode(bool flag) override;
        void send_event(const Event& event);
        void process_event();

        SDL_Surface* create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels);
        void destroy_icon();
        void destroy_cursor();

        // IMGUI
        WindowInterface& initialize_imgui() override;
        WindowInterface& terminate_imgui() override;
        WindowInterface& new_imgui_frame() override;
        void initialize_imgui_opengl();
        void initialize_imgui_vulkan();

        WindowInterface& update_monitor_info(MonitorInfo& info) override;


        ~WindowSDL();
    };
}// namespace Engine
