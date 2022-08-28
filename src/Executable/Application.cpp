#include <Application.hpp>
#include <Init/init.hpp>
#include <engine.hpp>


using namespace Engine;

Application::Application()
{
    // Window params
    init();
    window.init({1280, 720}, window_title, WIN_RESIZABLE);
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
