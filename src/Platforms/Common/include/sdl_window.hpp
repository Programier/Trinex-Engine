#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>
#include <Window/window.hpp>

namespace Engine
{
	struct ENGINE_EXPORT WindowSDL : public Window {
		using Window::m_size;

		Buffer m_icon_buffer;
		Buffer m_cursor_icon_buffer;

		SDL_Window* m_window       = nullptr;
		SDL_Surface* m_icon        = nullptr;
		SDL_Surface* m_cursor_icon = nullptr;
		SDL_Cursor* m_cursor       = nullptr;

		SDL_WindowFlags m_api;
		SDL_Event m_event;

		Identifier m_id;

		using Window::height;
		using Window::size;
		using Window::width;

		WindowSDL* sdl_initialize(const WindowConfig* config);
		WindowSDL& width(float_t width) override;
		WindowSDL& height(float_t height) override;
		WindowSDL& size(const Vector2u& size) override;
		String title() override;
		WindowSDL& title(const String& title) override;
		Vector2u position() override;
		WindowSDL& position(const Vector2u& position) override;
		bool resizable() override;
		WindowSDL& resizable(bool value) override;
		WindowSDL& focus() override;
		bool focused() override;
		WindowSDL& show() override;
		WindowSDL& hide() override;
		bool is_visible() override;
		bool is_iconify() override;
		WindowSDL& iconify() override;
		bool is_restored() override;
		WindowSDL& restore() override;
		WindowSDL& opacity(float value) override;
		float opacity() override;
		WindowSDL& icon(const Image& image) override;
		WindowSDL& cursor(const Image& image, Vector2i hotspot) override;
		WindowSDL& attribute(const WindowAttribute& attrib, bool value) override;
		bool attribute(const WindowAttribute& attrib) override;
		WindowSDL& cursor_mode(const CursorMode& mode) override;
		CursorMode cursor_mode() override;
		bool support_orientation(Orientation orientation) override;
		Identifier id() override;
		void* native_window() override;
		size_t monitor_index() override;

		SDL_Surface* create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels);
		void destroy_icon();
		void destroy_cursor();
		~WindowSDL();
	};
}// namespace Engine
