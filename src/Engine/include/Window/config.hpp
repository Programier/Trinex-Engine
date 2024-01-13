#pragma once
#include <Core/config.hpp>
#include <Window/window_interface.hpp>

namespace Engine
{
    struct ENGINE_EXPORT WindowConfig : public Config {
        Vector<WindowAttribute> attributes;

        String title;
        String api_name;
        String client;
        Size2D size;
        Vector<WindowOrientation> orientations;
        bool vsync;

        WindowConfig& update();
        WindowConfig& update_using_args();
    };

    extern ENGINE_EXPORT WindowConfig global_window_config;
}// namespace Engine
