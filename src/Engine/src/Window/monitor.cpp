#include <SDL.h>
#include <Window/monitor.hpp>
#include <engine.hpp>

namespace Engine
{
    // Monitor init
    namespace Monitor
    {
        SDL_DisplayMode mode;
        DPI display_dpi;

        void update()
        {
            SDL_GetCurrentDisplayMode(0, &mode);
            SDL_GetDisplayDPI(0, &display_dpi.ddpi, &display_dpi.hdpi, &display_dpi.vdpi);
        }


        int red_bits()
        {
            throw not_implemented;
        }

        int green_bits()
        {
            throw not_implemented;
        }

        int blue_bits()
        {
            throw not_implemented;
        }

        Size1D height()
        {
            return static_cast<Size1D>(mode.h);
        }

        Size1D width()
        {
            return static_cast<Size1D>(mode.w);
        }

        int refresh_rate()
        {
            return mode.refresh_rate;
        }

        Size2D size()
        {
            return {width(), height()};
        }

        const DPI& dpi()
        {
            return display_dpi;
        }
    }// namespace Monitor
}// namespace Engine
