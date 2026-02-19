#include <Core/logger.hpp>
#include <Core/reflection/struct.hpp>
#include <Image/image.hpp>
#include <Platform/platform.hpp>
#include <RHI/rhi.hpp>
#include <SDL_gamecontroller.h>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <Window/window_manager.hpp>
#include <sdl_window.hpp>


namespace Engine
{
	static const Map<WindowAttribute::Enum, SDL_WindowFlags> window_attributes = {
	        {WindowAttribute::Resizable, SDL_WINDOW_RESIZABLE},
	        {WindowAttribute::FullScreen, SDL_WINDOW_FULLSCREEN},
	        {WindowAttribute::Shown, SDL_WINDOW_SHOWN},
	        {WindowAttribute::Hidden, SDL_WINDOW_HIDDEN},
	        {WindowAttribute::BorderLess, SDL_WINDOW_BORDERLESS},
	        {WindowAttribute::MouseFocus, SDL_WINDOW_MOUSE_FOCUS},
	        {WindowAttribute::InputFocus, SDL_WINDOW_INPUT_FOCUS},
	        {WindowAttribute::InputGrabbed, SDL_WINDOW_INPUT_GRABBED},
	        {WindowAttribute::Minimized, SDL_WINDOW_MINIMIZED},
	        {WindowAttribute::Maximized, SDL_WINDOW_MAXIMIZED},
	        {WindowAttribute::MouseCapture, SDL_WINDOW_MOUSE_CAPTURE},
	        {WindowAttribute::MouseGrabbed, SDL_WINDOW_MOUSE_GRABBED},
	        {WindowAttribute::KeyboardGrabbed, SDL_WINDOW_KEYBOARD_GRABBED}};


#define has_flag(flag) static_cast<bool>(SDL_GetWindowFlags(m_window) & flag)
	static uint32_t to_sdl_attrib(const Set<WindowAttribute::Enum>& attrib)
	{
		uint32_t value = 0;
		for (auto ell : attrib)
		{
			value |= window_attributes.at(ell);
		}
		return value;
	}

	static SDL_WindowFlags sdl_api()
	{
		const Name api_name = rhi->info.struct_instance->name();
		if (api_name == "OPENGL")
		{
			return SDL_WINDOW_OPENGL;
		}
		else if (api_name == "VULKAN")
		{
			return SDL_WINDOW_VULKAN;
		}

		return {};
	}

	static int validate_pos(int pos)
	{
		return pos < 0 ? SDL_WINDOWPOS_CENTERED : pos;
	}

	WindowSDL* WindowSDL::sdl_initialize(const WindowConfig* info)
	{
		//m_vsync_status = info->vsync;
		uint32_t attrib = to_sdl_attrib(info->attributes);

		if ((attrib & SDL_WINDOW_SHOWN) != SDL_WINDOW_SHOWN && (attrib & SDL_WINDOW_HIDDEN) != SDL_WINDOW_HIDDEN)
		{
			attrib |= SDL_WINDOW_SHOWN;
		}

#if PLATFORM_LINUX
		SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif

		m_api = sdl_api();

		m_window = SDL_CreateWindow(info->title.c_str(), validate_pos(info->position.x), validate_pos(info->position.y),
		                            static_cast<int>(info->size.x), static_cast<int>(info->size.y),
		                            m_api | SDL_WINDOW_ALLOW_HIGHDPI | attrib);

		{
			int_t x, y;
			SDL_GetWindowSize(m_window, &x, &y);
			m_size = {static_cast<float>(x), static_cast<float>(y)};
		}

		m_id = static_cast<Identifier>(SDL_GetWindowID(m_window));

		trinex_verify_fmt(m_window, "Failed to initialize window: %s", SDL_GetError());
		return this;
	}

	WindowSDL& WindowSDL::width(float_t w)
	{
		size({w, height()});
		return *this;
	}

	WindowSDL& WindowSDL::height(float_t h)
	{
		size({width(), h});
		return *this;
	}

	WindowSDL& WindowSDL::size(const Vector2u& size)
	{
		SDL_SetWindowSize(m_window, size.x, size.y);
		return *this;
	}

	String WindowSDL::title()
	{
		return SDL_GetWindowTitle(m_window);
	}

	WindowSDL& WindowSDL::title(const String& title)
	{
		SDL_SetWindowTitle(m_window, title.c_str());
		return *this;
	}

	Vector2u WindowSDL::position()
	{
		int x, y;
		SDL_GetWindowPosition(m_window, &x, &y);
		size_t index = monitor_index();
		auto info    = Platform::monitor_info(index);
		return {x, info.size.y - (y + size().y)};
	}

	WindowSDL& WindowSDL::position(const Vector2u& position)
	{
		size_t index = monitor_index();
		auto info    = Platform::monitor_info(index);
		float new_y  = -position.y + info.size.y - size().y;
		SDL_SetWindowPosition(m_window, position.x, new_y);
		return *this;
	}

	bool WindowSDL::resizable()
	{
		return has_flag(SDL_WINDOW_RESIZABLE);
	}

	WindowSDL& WindowSDL::resizable(bool value)
	{
		SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(value));
		return *this;
	}

	WindowSDL& WindowSDL::focus()
	{
		SDL_SetWindowInputFocus(m_window);
		return *this;
	}

	bool WindowSDL::focused()
	{
		return has_flag(SDL_WINDOW_INPUT_FOCUS);
	}

	WindowSDL& WindowSDL::show()
	{
		SDL_ShowWindow(m_window);
		return *this;
	}

	WindowSDL& WindowSDL::hide()
	{
		SDL_HideWindow(m_window);
		return *this;
	}

	bool WindowSDL::is_visible()
	{
		return has_flag(SDL_WINDOW_SHOWN);
	}

	bool WindowSDL::is_iconify()
	{
		return has_flag(SDL_WINDOW_MINIMIZED);
	}

	WindowSDL& WindowSDL::iconify()
	{
		SDL_MinimizeWindow(m_window);
		return *this;
	}

	bool WindowSDL::is_restored()
	{
		return !is_iconify();
	}

	WindowSDL& WindowSDL::restore()
	{
		SDL_RestoreWindow(m_window);
		return *this;
	}

	WindowSDL& WindowSDL::opacity(float value)
	{
		SDL_SetWindowOpacity(m_window, value);
		return *this;
	}

	float WindowSDL::opacity()
	{
		float o;
		SDL_GetWindowOpacity(m_window, &o);
		return o;
	}

	void WindowSDL::destroy_icon()
	{
		if (m_icon)
		{
			SDL_FreeSurface(m_icon);

			m_icon = nullptr;
		}
	}

	void WindowSDL::destroy_cursor()
	{
		if (m_cursor)
		{
			SDL_FreeCursor(m_cursor);
			m_cursor = nullptr;
		}

		if (m_cursor_icon)
		{
			SDL_FreeSurface(m_cursor_icon);
			m_cursor_icon = nullptr;
		}
	}

	SDL_Surface* WindowSDL::create_surface(const Buffer& buffer, int_t width, int_t height, int_t channels)
	{

		if (buffer.empty())
			return nullptr;
		void* data = (void*) buffer.data();

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		int_t r_mask = 0x000000FF;
		int_t g_mask = 0x0000FF00;
		int_t b_mask = 0x00FF0000;
		int_t a_mask = (channels == 4) ? 0xFF000000 : 0;
#else
		int_t s      = (channels == 4) ? 0 : 8;
		int_t r_mask = 0xFF000000 >> s;
		int_t g_mask = 0x00FF0000 >> s;
		int_t b_mask = 0x0000FF00 >> s;
		int_t a_mask = 0x000000FF >> s;
#endif
		SDL_Surface* surface =
		        SDL_CreateRGBSurfaceFrom(data, width, height, channels * 8, width * channels, r_mask, g_mask, b_mask, a_mask);
		if (surface == nullptr)
		{
			error_log("WindowSDL", "Failed to create surface from image: %s", SDL_GetError());
		}
		return surface;
	}

	WindowSDL& WindowSDL::icon(const Image& image)
	{
		int_t channels = image.channels();
		if (channels == 3 || channels == 4)
		{
			destroy_icon();

			m_icon_buffer = image.buffer();
			m_icon        = create_surface(m_icon_buffer, image.width(), image.height(), channels);

			SDL_SetWindowIcon(m_window, m_icon);
		}
		else
		{
			error_log("WindowSDL", "Window icon format must be RGB or RGBA!");
		}

		return *this;
	}

	WindowSDL& WindowSDL::cursor(const Image& image, Vector2i hotspot)
	{
		int_t channels = image.channels();
		if (channels == 3 || channels == 4)
		{
			destroy_cursor();

			m_cursor_icon_buffer = image.buffer();
			m_cursor_icon        = create_surface(m_cursor_icon_buffer, image.width(), image.height(), channels);

			if (m_cursor_icon)
			{
				m_cursor = SDL_CreateColorCursor(m_cursor_icon, hotspot.x, hotspot.y);
			}

			SDL_SetCursor(m_cursor);
		}
		else
		{
			error_log("WindowSDL", "Window icon format must be RGB or RGBA!");
		}

		return *this;
	}

	WindowSDL& WindowSDL::attribute(const WindowAttribute& attrib, bool value)
	{

		static struct {
			bool m_fullscreen = false;
			Uint32 m_flag     = 0;
		} fullscreen_mode;

		fullscreen_mode.m_fullscreen = false;
		fullscreen_mode.m_flag       = 0;

		switch (attrib)
		{
			case WindowAttribute::Resizable: SDL_SetWindowResizable(m_window, static_cast<SDL_bool>(value)); break;

			case WindowAttribute::FullScreen:
				fullscreen_mode.m_flag       = value ? SDL_WINDOW_FULLSCREEN : 0;
				fullscreen_mode.m_fullscreen = true;
				break;

			case WindowAttribute::Shown:
			{
				value ? show() : hide();
				break;
			}
			case WindowAttribute::Hidden:
			{
				value ? hide() : show();
				break;
			}
			case WindowAttribute::BorderLess: SDL_SetWindowBordered(m_window, static_cast<SDL_bool>(value)); break;

			case WindowAttribute::InputFocus:
			{
				if (value)
					SDL_SetWindowInputFocus(m_window);
				break;
			}

			case WindowAttribute::Minimized:
			{
				if (value)
					SDL_MinimizeWindow(m_window);
				break;
			}

			case WindowAttribute::Maximized:
			{
				if (value)
					SDL_MaximizeWindow(m_window);
				break;
			}
			case WindowAttribute::MouseGrabbed:
			{
				SDL_SetWindowMouseGrab(m_window, static_cast<SDL_bool>(value));
				break;
			}

			case WindowAttribute::KeyboardGrabbed:
			{
				SDL_SetWindowKeyboardGrab(m_window, static_cast<SDL_bool>(value));
				break;
			}

			default: break;
		}


		if (fullscreen_mode.m_fullscreen)
			SDL_SetWindowFullscreen(m_window, fullscreen_mode.m_flag);

		return *this;
	}

	bool WindowSDL::attribute(const WindowAttribute& attrib)
	{
		auto f = window_attributes.at(attrib);
		return has_flag(f);
	}

	WindowSDL& WindowSDL::cursor_mode(const CursorMode& mode)
	{
		SDL_ShowCursor((mode == CursorMode::Hidden ? SDL_DISABLE : SDL_ENABLE));
		return *this;
	}

	CursorMode WindowSDL::cursor_mode()
	{
		int result = SDL_ShowCursor(SDL_QUERY);

		if (result == SDL_ENABLE)
			return CursorMode::Normal;
		return CursorMode::Hidden;
	}

	bool WindowSDL::support_orientation(Orientation orientations)
	{
		//        static Map<WindowOrientation, const char*> m_orientation_map = {
		//                {WindowOrientation::Landscape, "LandscapeRight"},
		//                {WindowOrientation::LandscapeFlipped, "LandscapeLeft"},
		//                {WindowOrientation::Portrait, "Portrait"},
		//                {WindowOrientation::PortraitFlipped, "PortraitUpsideDown"}};


		//        String result;
		//        for (auto ell : orientations)
		//        {
		//            if (!result.empty())
		//                result += " ";
		//            result += m_orientation_map.at(ell);
		//        }

		//        SDL_SetHint(SDL_HINT_ORIENTATIONS, result.c_str());
		return true;
	}

	template<typename Type>
	using ValueMap = const Map<Uint8, Type>;


	template<typename MapType, typename KeyType, bool& result>
	static decltype(auto) value_at(MapType&& map, KeyType&& key)
	{
		result = true;
		return map.at(std::forward<KeyType>(key));
	}

	Identifier WindowSDL::id()
	{
		return m_id;
	}

	void* WindowSDL::native_window()
	{
		return m_window;
	}

	size_t WindowSDL::monitor_index()
	{
		return static_cast<size_t>(SDL_GetWindowDisplayIndex(m_window));
	}

	WindowSDL::~WindowSDL()
	{
		if (m_window)
		{
			destroy_icon();
			destroy_cursor();
			SDL_DestroyWindow(m_window);
			m_window = 0;
		}
	}
}// namespace Engine
