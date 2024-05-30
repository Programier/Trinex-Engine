#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <Window/window_interface.hpp>
#include <Window/window_manager_interface.hpp>
#include <imgui.h>

namespace Engine
{
    union SDL_ApiContext
    {
        void* opengl_context = nullptr;
        VkSurfaceKHR vulkan_surface;
    };

    struct WindowSDL : public WindowInterface {
        Set<void (*)(SDL_Event*)> m_on_event;

        Buffer m_icon_buffer;
        Buffer m_cursor_icon_buffer;

        SDL_Window* m_window       = nullptr;
        SDL_Surface* m_icon        = nullptr;
        SDL_Surface* m_cursor_icon = nullptr;
        SDL_Cursor* m_cursor       = nullptr;

        SDL_ApiContext created_context;
        SDL_ApiContext binded_context;

        SDL_WindowFlags m_api;
        CursorMode m_c_mode;
        SDL_Event m_event;

        Identifier m_id;

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
        WindowInterface& present() override;
        Vector<String> required_extensions() override;
        Identifier id() override;

        void* create_api_context(const char* any_text, ...) override;
        void bind_api_context(void* context) override;
        WindowInterface& make_current() override;
        WindowInterface& destroy_api_context() override;


        SDL_Surface* create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels);
        void destroy_icon();
        void destroy_cursor();
        WindowSDL& vsync(bool) override;
        bool vsync() override;

        // IMGUI
        WindowInterface& initialize_imgui() override;
        WindowInterface& terminate_imgui() override;
        WindowInterface& new_imgui_frame() override;
        void initialize_imgui_opengl();
        void initialize_imgui_vulkan();

        ~WindowSDL();
    };
}// namespace Engine
