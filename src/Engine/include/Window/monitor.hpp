#pragma once
#include <BasicFunctional/engine_types.hpp>

namespace Engine
{
    struct DPI
    {
        float ddpi;
        float hdpi;
        float vdpi;
    };

    namespace Monitor
    {
        void* monitor();
        int red_bits();
        int green_bits();
        int blue_bits();
        Size1D height();
        Size1D width();
        int refresh_rate();
        Size2D size();
        void update();
        const DPI& dpi();
    }// namespace
}
