#pragma once
#include <Window/window.hpp>
#include <panel.hpp>

namespace Editor
{
    class Application
    {
        Engine::Window _M_window;
        std::vector<Panel*> _M_panels;

    public:
        Application();
        Application& loop();

        ~Application();
    };
}
