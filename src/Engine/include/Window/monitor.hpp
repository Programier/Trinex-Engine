#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT DPI
    {
        float ddpi;
        float hdpi;
        float vdpi;
    };

    namespace Monitor
    {
        ENGINE_EXPORT void* monitor();
        ENGINE_EXPORT uint_t height();
        ENGINE_EXPORT uint_t width();
        ENGINE_EXPORT int_t refresh_rate();
        ENGINE_EXPORT Size2D size();
        ENGINE_EXPORT void update();
        ENGINE_EXPORT const DPI& dpi();
        ENGINE_EXPORT Size2D physical_size(PhysicalSizeMetric metric = PhysicalSizeMetric::Inch);
    }// namespace
}
