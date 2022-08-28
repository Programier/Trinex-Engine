#pragma once
#include "Window/window.hpp"
#include <list>
#include <GUI.hpp>

namespace Engine
{

    class Application
    {
    private:
        bool _M_gui = false;

    public:
        std::list<void (*)()> additional_funcs;
        Window window;
        const Size2D& window_size = window.size();
        glm::vec4 background_color;
        char window_title[100] = "Engine";

    public:
        Application();
        Application& loop();
        Application& init_gui();


        ~Application();
    };
}// namespace Engine
