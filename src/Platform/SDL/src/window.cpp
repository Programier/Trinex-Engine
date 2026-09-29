#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDLPlatform/window.hpp>

namespace Trinex::Platform
{
	static SDL_WindowFlags window_flags_of(WindowAttribute attributes)
	{
		SDL_WindowFlags flags = 0;

		while (attributes)
		{
			const WindowAttribute attribute = attributes.first();
			attributes.remove(attribute);

			switch (attribute)
			{
				case WindowAttribute::Fullscreen: flags |= SDL_WINDOW_FULLSCREEN; break;
				case WindowAttribute::Hidden: flags |= SDL_WINDOW_HIDDEN; break;
				case WindowAttribute::Borderless: flags |= SDL_WINDOW_BORDERLESS; break;
				case WindowAttribute::Resizable: flags |= SDL_WINDOW_RESIZABLE; break;
				case WindowAttribute::Minimized: flags |= SDL_WINDOW_MINIMIZED; break;
				case WindowAttribute::Maximized: flags |= SDL_WINDOW_MAXIMIZED; break;
				case WindowAttribute::Occluded: flags |= SDL_WINDOW_OCCLUDED; break;
				case WindowAttribute::InputFocus: flags |= SDL_WINDOW_INPUT_FOCUS; break;
				case WindowAttribute::MouseFocus: flags |= SDL_WINDOW_MOUSE_FOCUS; break;
				case WindowAttribute::MouseGrabbed: flags |= SDL_WINDOW_MOUSE_GRABBED; break;
				case WindowAttribute::KeyboardGrabbed: flags |= SDL_WINDOW_KEYBOARD_GRABBED; break;
				case WindowAttribute::MouseCapture: flags |= SDL_WINDOW_MOUSE_CAPTURE; break;
				case WindowAttribute::MouseRelativeMode: flags |= SDL_WINDOW_MOUSE_RELATIVE_MODE; break;
				case WindowAttribute::AlwaysOnTop: flags |= SDL_WINDOW_ALWAYS_ON_TOP; break;
				case WindowAttribute::HighPixelDensity: flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY; break;
				case WindowAttribute::Transparent: flags |= SDL_WINDOW_TRANSPARENT; break;
				case WindowAttribute::NotFocusable: flags |= SDL_WINDOW_NOT_FOCUSABLE; break;
				case WindowAttribute::Modal: flags |= SDL_WINDOW_MODAL; break;
				case WindowAttribute::Utility: flags |= SDL_WINDOW_UTILITY; break;
				case WindowAttribute::Tooltip: flags |= SDL_WINDOW_TOOLTIP; break;
				case WindowAttribute::PopupMenu: flags |= SDL_WINDOW_POPUP_MENU; break;
				case WindowAttribute::External: flags |= SDL_WINDOW_EXTERNAL; break;

				default: break;
			}
		}

		return flags;
	}

	static WindowAttribute window_attributes_of(SDL_WindowFlags flags)
	{
		WindowAttribute attributes = WindowAttribute::Undefined;

		if (flags & SDL_WINDOW_FULLSCREEN)
			attributes |= WindowAttribute::Fullscreen;

		if (flags & SDL_WINDOW_HIDDEN)
			attributes |= WindowAttribute::Hidden;

		if (flags & SDL_WINDOW_BORDERLESS)
			attributes |= WindowAttribute::Borderless;

		if (flags & SDL_WINDOW_RESIZABLE)
			attributes |= WindowAttribute::Resizable;

		if (flags & SDL_WINDOW_MINIMIZED)
			attributes |= WindowAttribute::Minimized;

		if (flags & SDL_WINDOW_MAXIMIZED)
			attributes |= WindowAttribute::Maximized;

		if (flags & SDL_WINDOW_OCCLUDED)
			attributes |= WindowAttribute::Occluded;

		if (flags & SDL_WINDOW_INPUT_FOCUS)
			attributes |= WindowAttribute::InputFocus;

		if (flags & SDL_WINDOW_MOUSE_FOCUS)
			attributes |= WindowAttribute::MouseFocus;

		if (flags & SDL_WINDOW_MOUSE_GRABBED)
			attributes |= WindowAttribute::MouseGrabbed;

		if (flags & SDL_WINDOW_KEYBOARD_GRABBED)
			attributes |= WindowAttribute::KeyboardGrabbed;

		if (flags & SDL_WINDOW_MOUSE_CAPTURE)
			attributes |= WindowAttribute::MouseCapture;

		if (flags & SDL_WINDOW_MOUSE_RELATIVE_MODE)
			attributes |= WindowAttribute::MouseRelativeMode;

		if (flags & SDL_WINDOW_ALWAYS_ON_TOP)
			attributes |= WindowAttribute::AlwaysOnTop;

		if (flags & SDL_WINDOW_HIGH_PIXEL_DENSITY)
			attributes |= WindowAttribute::HighPixelDensity;

		if (flags & SDL_WINDOW_TRANSPARENT)
			attributes |= WindowAttribute::Transparent;

		if (flags & SDL_WINDOW_NOT_FOCUSABLE)
			attributes |= WindowAttribute::NotFocusable;

		if (flags & SDL_WINDOW_MODAL)
			attributes |= WindowAttribute::Modal;

		if (flags & SDL_WINDOW_UTILITY)
			attributes |= WindowAttribute::Utility;

		if (flags & SDL_WINDOW_TOOLTIP)
			attributes |= WindowAttribute::Tooltip;

		if (flags & SDL_WINDOW_POPUP_MENU)
			attributes |= WindowAttribute::PopupMenu;

		if (flags & SDL_WINDOW_EXTERNAL)
			attributes |= WindowAttribute::External;

		return attributes;
	}

	SDLWindow::SDLWindow(SDL_Window* window) : m_window(window) {}

	SDLWindow::~SDLWindow()
	{
		destroy_window();
	}

	void SDLWindow::destroy_window()
	{
		if (m_window)
		{
			SDLWindowSystem::instance()->unbind(id());
			m_window = nullptr;
		}
	}

	u32 SDLWindow::monitor() const
	{
		if (m_window == nullptr)
			return 0;

		return static_cast<u32>(SDL_GetDisplayForWindow(m_window));
	}

	u32 SDLWindow::id() const
	{
		if (m_window == nullptr)
			return 0;

		return SDL_GetWindowID(m_window);
	}

	Ref<Window> SDLWindow::parent() const
	{
		if (SDL_Window* parent = SDL_GetWindowParent(m_window))
		{
			return find(SDL_GetWindowID(parent));
		}

		return nullptr;
	}

	bool SDLWindow::parent(Window* window)
	{
		if (m_window == nullptr)
			return false;

		SDL_Window* parent = nullptr;

		if (window)
		{
			parent = static_cast<SDLWindow*>(window)->m_window;
		}

		return SDL_SetWindowParent(m_window, parent);
	}

	Vector2u SDLWindow::size() const
	{
		int w, h;

		if (m_window && SDL_GetWindowSize(m_window, &w, &h))
		{
			return {w, h};
		}

		return {0, 0};
	}

	bool SDLWindow::size(Vector2u size)
	{
		if (m_window == nullptr)
			return false;

		return SDL_SetWindowSize(m_window, size.x, size.y);
	}

	Vector2i SDLWindow::position() const
	{
		if (m_window == nullptr)
			return {0, 0};

		int x, y;
		if (SDL_GetWindowPosition(m_window, &x, &y))
			return {x, y};

		return {0, 0};
	}

	bool SDLWindow::position(Vector2i position)
	{
		if (m_window == nullptr)
			return false;

		return SDL_SetWindowPosition(m_window, position.x, position.y);
	}

	const char* SDLWindow::title() const
	{
		return SDL_GetWindowTitle(m_window);
	}

	bool SDLWindow::title(const char* title)
	{
		return SDL_SetWindowTitle(m_window, title);
	}

	WindowAttribute SDLWindow::attributes(WindowAttribute mask) const
	{
		if (m_window == nullptr)
			return {};

		return window_attributes_of(SDL_GetWindowFlags(m_window)) & mask;
	}

	WindowAttribute SDLWindow::attributes(WindowAttribute mask, bool status)
	{
		if (m_window == nullptr)
			return {};

		WindowAttribute changed;

		while (mask)
		{
			const WindowAttribute attribute = mask.first();
			mask.remove(attribute);

			bool success = false;

			switch (attribute)
			{
				case WindowAttribute::Fullscreen: success = SDL_SetWindowFullscreen(m_window, status); break;
				case WindowAttribute::Hidden: success = status ? SDL_HideWindow(m_window) : SDL_ShowWindow(m_window); break;
				case WindowAttribute::Borderless: success = SDL_SetWindowBordered(m_window, !status); break;
				case WindowAttribute::Resizable: success = SDL_SetWindowResizable(m_window, status); break;
				case WindowAttribute::Minimized:
					success = status ? SDL_MinimizeWindow(m_window) : SDL_RestoreWindow(m_window);
					break;

				case WindowAttribute::Maximized:
					success = status ? SDL_MaximizeWindow(m_window) : SDL_RestoreWindow(m_window);
					break;

				case WindowAttribute::MouseGrabbed: success = SDL_SetWindowMouseGrab(m_window, status); break;
				case WindowAttribute::KeyboardGrabbed: success = SDL_SetWindowKeyboardGrab(m_window, status); break;
				case WindowAttribute::MouseRelativeMode: success = SDL_SetWindowRelativeMouseMode(m_window, status); break;
				case WindowAttribute::AlwaysOnTop: success = SDL_SetWindowAlwaysOnTop(m_window, status); break;
				case WindowAttribute::NotFocusable: success = SDL_SetWindowFocusable(m_window, !status); break;
				case WindowAttribute::Modal: success = SDL_SetWindowModal(m_window, status); break;

				default: break;
			}

			if (success)
				changed |= attribute;
		}

		return changed;
	}

	Vector2u SDLWindow::minimum_size() const
	{
		if (m_window == nullptr)
			return {0, 0};

		int x, y;
		if (SDL_GetWindowMinimumSize(m_window, &x, &y))
			return {x, y};

		return {0, 0};
	}

	bool SDLWindow::minimum_size(Vector2u size)
	{
		if (m_window == nullptr)
			return false;

		return SDL_SetWindowMinimumSize(m_window, size.x, size.y);
	}

	Vector2u SDLWindow::maximum_size() const
	{
		if (m_window == nullptr)
			return {0, 0};

		int x, y;
		if (SDL_GetWindowMaximumSize(m_window, &x, &y))
			return {x, y};

		return {0, 0};
	}

	bool SDLWindow::maximum_size(Vector2u size)
	{
		if (m_window == nullptr)
			return false;

		return SDL_SetWindowMaximumSize(m_window, size.x, size.y);
	}

	f32 SDLWindow::density() const
	{
		if (m_window == nullptr)
			return false;

		return SDL_GetWindowPixelDensity(m_window);
	}

	bool SDLWindow::raise()
	{
		if (m_window == nullptr)
			return false;

		return SDL_RaiseWindow(m_window);
	}

	bool SDLWindow::restore()
	{
		if (m_window == nullptr)
			return false;

		return SDL_RestoreWindow(m_window);
	}

	f32 SDLWindow::opacity() const
	{
		if (m_window == nullptr)
			return 0.f;

		return SDL_GetWindowOpacity(m_window);
	}

	bool SDLWindow::opacity(f32 opacity)
	{
		if (m_window == nullptr)
			return false;

		return SDL_SetWindowOpacity(m_window, opacity);
	}

	bool SDLWindow::icon(const Image& image)
	{
		if (m_window == nullptr)
			return false;
		return false;
	}

	void* SDLWindow::native_handle() const
	{
		return m_window;
	}

	SDLWindowSystem* SDLWindowSystem::instance()
	{
		static SDLWindowSystem system;
		return &system;
	}

	Ref<Window> SDLWindowSystem::bind(SDL_Window* window)
	{
		auto ref             = Ref<SDLWindow>::make(window);
		m_windows[ref->id()] = ref.value();
		return ref;
	}

	SDLWindowSystem& SDLWindowSystem::unbind(u32 id)
	{
		m_windows.erase(id);
		return *this;
	}

	Ref<Window> SDLWindowSystem::create(const char* title, u32 width, u32 height, WindowAttribute flags)
	{
		if (SDL_Window* window = SDL_CreateWindow(title, width, height, window_flags_of(flags)))
		{
			return bind(window);
		}

		return {};
	}

	Ref<Window> SDLWindowSystem::grabbed() const
	{
		if (SDL_Window* window = SDL_GetGrabbedWindow())
		{
			return find(SDL_GetWindowID(window));
		}

		return {};
	}

	Ref<Window> SDLWindowSystem::find(u32 id) const
	{
		auto it = m_windows.find(id);

		if (it == m_windows.end())
			return {};

		return retain_ref(it->second);
	}
}// namespace Trinex::Platform
