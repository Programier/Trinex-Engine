#include <Application.hpp>
#include <Core/logger.hpp>


namespace Engine
{
    GameApplication::GameApplication()
    {
        this->init_info.api = EngineAPI::Vulkan;
        this->init_info.window_name = STR("Engine");
        this->init_info.window_size = Size2D(1280, 720);
        this->init_info.window_attribs |= WindowAttrib::WIN_RESIZABLE;
        init();
    }

    GameApplication& GameApplication::on_init()
    {
        return *this;
    }

    GameApplication& GameApplication::on_render_frame()
    {
        if (Event::keyboard.just_pressed() == Engine::KEY_SPACE)
            window.vsync(!window.vsync());

        _M_max_fps = glm::max(_M_max_fps, 1.f / float(Event::diff_time()));
        return *this;
    }

    GameApplication::~GameApplication()
    {
        logger->log("Max FPS: %f", _M_max_fps);
    }
}// namespace Engine


#include <string>
void test();
int game_main(int argc, char* argv[])
try
{
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--test")
            test();
    Engine::GameApplication app;
    app.start();
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log("%s\n", e.what());
    return 1;
}
