#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <Window/window_interface.hpp>
#include <Window/window_manager_interface.hpp>
#include <imgui.h>

namespace Engine
{
    struct WindowSDL : public WindowInterface {
        Set<void (*)(SDL_Event*)> _M_on_event;

        Buffer _M_icon_buffer;
        Buffer _M_cursor_icon_buffer;

        SDL_Window* _M_window       = nullptr;
        SDL_Surface* _M_icon        = nullptr;
        SDL_Surface* _M_cursor_icon = nullptr;
        SDL_Cursor* _M_cursor       = nullptr;

        SDL_GLContext _M_gl_context;
        ImGuiContext* _M_imgui_context = nullptr;
        SDL_WindowFlags _M_api;
        CursorMode _M_c_mode;
        VkSurfaceKHR _M_vulkan_surface;
        SDL_Event _M_event;

        Identifier _M_id;

        bool _M_vsync_status;

        WindowInterface* initialize(const WindowConfig* config);
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
        bool resizable() override;
        WindowInterface& resizable(bool value) override;
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
        bool support_orientation(WindowOrientation orientation) override;
        void* create_surface(const char* any_text, ...) override;
        WindowInterface& present() override;
        Vector<const char*> required_extensions() override;
        Identifier id() override;
        WindowInterface& make_current(void* context) override;

        int_t create_message_box(const MessageBoxCreateInfo& info) override;
        SDL_Surface* create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels);
        void destroy_icon();
        void destroy_cursor();
        WindowSDL& vsync(bool) override;
        bool vsync() override;

        // IMGUI
        WindowInterface& initialize_imgui(ImGuiContext* ctx) override;
        WindowInterface& terminate_imgui() override;
        WindowInterface& new_imgui_frame() override;
        void initialize_imgui_opengl();
        void initialize_imgui_vulkan();

        ~WindowSDL();
    };
}// namespace Engine
