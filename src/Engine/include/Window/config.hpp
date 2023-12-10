#pragma once
#include <Core/config.hpp>
#include <Window/window_interface.hpp>

namespace Engine
{
    struct WindowConfig {
        Vector<WindowAttribute> attributes;

        String title;
        String api_name;
        String client;
        Size2D size;
        Vector<WindowOrientation> orientations;
        bool vsync;

        WindowConfig& update();
    };

    extern ENGINE_EXPORT WindowConfig global_window_config;
}// namespace Engine
