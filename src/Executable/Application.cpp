#include <Application.hpp>
#include <Init/init.hpp>
#include <engine.hpp>
#include <iostream>


using namespace Engine;

Application::Application()
{
    // Window params
    init();
    window.init(Monitor::size() / 2.f, window_title, WIN_RESIZABLE);
}


Application& Application::loop()
{
    while (window.is_open())
    {
        window.clear_buffer();
        for (auto& func : additional_funcs) func();

        if (_M_gui)
            GUI::render();

        if (window.event.keyboard.just_pressed() == KEY_SPACE)
        {
            init_gui();
        }

        window.swap_buffers();
        window.event.poll_events();
    }
    return *this;
}

Application& Application::init_gui()
{
    if (_M_gui == false)
        GUI::init(this);
    else
        GUI::terminate();
    _M_gui = !_M_gui;
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
