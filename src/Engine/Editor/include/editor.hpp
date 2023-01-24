#pragma once
#include <Window/window.hpp>
#include <panel.hpp>
#include <unordered_set>
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
        std::vector<Panel*> _M_panels;

    public:
        std::unordered_set<Command> commands;

        Application();
        Application& loop();

        ~Application();
    };

    extern Application* application;
}// namespace Editor
