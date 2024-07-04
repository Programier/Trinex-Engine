#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <Window/window.hpp>
#include <windows.h>

namespace Engine
{
    HWND extract_d3dx11_hwnd(class Window* main_window)
    {
        SDL_Window* window = reinterpret_cast<SDL_Window*>(main_window->native_window());
        SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        SDL_GetWindowWMInfo(window, &info);
        return info.info.win.window;
    }
}// namespace Engine
