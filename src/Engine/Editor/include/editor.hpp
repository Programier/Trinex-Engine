#pragma once
#include <Window/window.hpp>
#include <panel.hpp>

namespace Editor
{

    enum class Command
    {
        ShowDepth,
        OctreeRender,
        ObjectChanged,
    };

    class Application
    {
        Engine::Window _M_window;
        Vector<Panel*> _M_panels;

    public:
        Set<Command> commands;

        Application();
        Application& loop();

        ~Application();
    };

    extern Application* application;
}// namespace Editor
