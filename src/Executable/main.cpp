#include <Application.hpp>
#include <GUI.hpp>
#include <Init/init.hpp>
#include <Window/color.hpp>
#include <Window/window.hpp>
#include <engine.hpp>
#include <glm/ext.hpp>
#include <imgui.h>
#include <iostream>
#include <list>

using namespace Engine;


Application* parse_args(int argc, char** argv, Application* app)
{

    for (int i = 1; i < argc; i++)
    {
        std::string param(argv[i]);
        if (param == "--imgui")
        {
            app->init_gui();
        }
    }
    return app;
}

int main(int argc, char** argv)
try
{
    delete &parse_args(argc, argv, new Application())->loop();
}
catch (const std::exception& e)
{
    std::clog << e.what() << std::endl;
}
