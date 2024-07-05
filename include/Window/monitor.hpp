#pragma once
#include <Core/engine_types.hpp>
#include <Core/enums.hpp>

namespace Engine
{
    struct ENGINE_EXPORT DPI {
        float ddpi;
        float hdpi;
        float vdpi;
    };

    struct MonitorInfo {
        DPI dpi;
        int_t width;
        int_t height;
        int_t refresh_rate;
    };

    namespace Monitor
    {
        ENGINE_EXPORT uint_t height();
        ENGINE_EXPORT uint_t width();
        ENGINE_EXPORT int_t refresh_rate();
        ENGINE_EXPORT Size2D size();
        ENGINE_EXPORT DPI dpi();
        ENGINE_EXPORT MonitorInfo info();
        ENGINE_EXPORT Size2D physical_size(PhysicalSizeMetric metric = PhysicalSizeMetric::Inch);
    }// namespace Monitor
}// namespace Engine
