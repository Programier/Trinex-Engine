#pragma once
#include <Core/engine_types.hpp>
#include <Core/export.hpp>

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
        ENGINE_EXPORT Size1D height();
        ENGINE_EXPORT Size1D width();
        ENGINE_EXPORT int_t refresh_rate();
        ENGINE_EXPORT Size2D size();
        ENGINE_EXPORT void update();
        ENGINE_EXPORT const DPI& dpi();
    }// namespace
}
