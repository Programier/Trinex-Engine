#include <Core/base_engine.hpp>
#include <Window/monitor.hpp>
#include <Window/window_manager.hpp>

namespace Engine
{
    // Monitor init
    namespace Monitor
    {
        ENGINE_EXPORT uint_t height()
        {
            return info().height;
        }

        ENGINE_EXPORT uint_t width()
        {
            return info().width;
        }

        ENGINE_EXPORT int_t refresh_rate()
        {
            return info().refresh_rate;
        }

        ENGINE_EXPORT Size2D size()
        {
            auto monitor_info = info();
            return {static_cast<float>(monitor_info.width), static_cast<float>(monitor_info.height)};
        }

        ENGINE_EXPORT DPI dpi()
        {
            return info().dpi;
        }

        ENGINE_EXPORT MonitorInfo info()
        {
            MonitorInfo monitor_info;
            WindowManager* instance = WindowManager::instance();
            if (instance)
            {
                instance->update_monitor_info(monitor_info);
            }
            return monitor_info;
        }

        ENGINE_EXPORT Size2D physical_size(PhysicalSizeMetric metric)
        {
            auto monitor_info = info();
            Size2D inches     = {static_cast<float>(monitor_info.width), static_cast<float>(monitor_info.height)};
            inches.x /= monitor_info.dpi.hdpi;
            inches.y /= monitor_info.dpi.vdpi;

            if (metric == PhysicalSizeMetric::Сentimeters)
            {
                inches *= 2.54f;
            }

            return inches;
        }
    }// namespace Monitor
}// namespace Engine
