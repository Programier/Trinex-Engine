#pragma once
#include <Window/window_interface.hpp>

namespace Engine
{
    struct ENGINE_EXPORT WindowConfig {
        Vector<WindowAttribute> attributes;
        Vector<WindowOrientation> orientations;

        String title;
        String api_name;
        String client;
        Size2D size;
        Point2D position = {-1, -1};

        bool vsync;

        WindowConfig();
        WindowConfig& initialize();
    };
}// namespace Engine
