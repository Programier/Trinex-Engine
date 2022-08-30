#include <Application.hpp>
#include <Init/init.hpp>
#include <engine.hpp>
#include <iostream>


using namespace Engine;

Application::Application()
{
    // Window params
    init();
    window.init(Monitor::size(), window_title, WIN_RESIZABLE | WIN_FULLSCREEN_DESKTOP);
}


Application& Application::loop()
{
    while (window.is_open())
    {
        window.clear_buffer();
        for (auto& func : additional_funcs) func();

        if (_M_gui)
            GUI::render();

        window.swap_buffers();
        window.event.poll_events();
    }
    return *this;
}

Application& Application::init_gui()
{
    GUI::init(this);
    _M_gui = true;
    return *this;
}


Application::~Application()
{
    if (_M_gui)
        GUI::terminate();
}


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

int game_main(int argc, char* argv[])
try
{
    delete &parse_args(argc, argv, new Application())->loop();
    return 0;
}
catch (const std::exception& e)
{
    std::clog << e.what() << std::endl;
}
