#include <Core/engine.hpp>
#include <SDL.h>
#include <Window/monitor.hpp>

namespace Engine
{
    // Monitor init
    namespace Monitor
    {
        SDL_DisplayMode mode;
        DPI display_dpi;

        ENGINE_EXPORT void update()
        {
            SDL_GetCurrentDisplayMode(0, &mode);
            SDL_GetDisplayDPI(0, &display_dpi.ddpi, &display_dpi.hdpi, &display_dpi.vdpi);
        }

        ENGINE_EXPORT uint_t height()
        {
            return static_cast<uint_t>(mode.h);
        }

        ENGINE_EXPORT uint_t width()
        {
            return static_cast<uint_t>(mode.w);
        }

        ENGINE_EXPORT int_t refresh_rate()
        {
            return mode.refresh_rate;
        }

        ENGINE_EXPORT Size2D size()
        {
            return {width(), height()};
        }

        ENGINE_EXPORT const DPI& dpi()
        {
            return display_dpi;
        }

        ENGINE_EXPORT Size2D physical_size(PhysicalSizeMetric metric)
        {
            Size2D inches = size();
            inches.x /= display_dpi.hdpi;
            inches.y /= display_dpi.vdpi;

            if (metric == PhysicalSizeMetric::Сentimeters)
            {
                inches *= 2.54f;
            }

            return inches;
        }
    }// namespace Monitor
}// namespace Engine
