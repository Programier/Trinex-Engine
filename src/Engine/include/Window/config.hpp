#pragma once
#include <Core/config.hpp>
#include <Window/window_interface.hpp>

namespace Engine
{
    struct WindowConfig {
        String title;
        String api_name;
        Size2D size;
        Vector<WindowAttribute> attributes;

        WindowConfig& update();
    };

    extern ENGINE_EXPORT WindowConfig global_window_config;
}// namespace Engine
