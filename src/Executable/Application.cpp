#include <Application.hpp>
#include <Core/decode_typeid_name.hpp>
#include <Core/etl/average.hpp>
#include <Core/file_manager.hpp>
#include <Core/logger.hpp>
#include <Core/mapped_memory.hpp>
#include <Core/package.hpp>
#include <Core/shader_types.hpp>

#include <Core/class.hpp>
#include <Core/etl/type_traits.hpp>
#include <Graphics/transform_component.hpp>
#include <Window/monitor.hpp>
#include <iostream>


namespace Engine
{
    GameApplication::GameApplication(int argc, char** argv)
    {
        this->init_info.window_name = STR("Engine");
        this->init_info.window_size = Size2D(1280, 720);
        this->init_info.window_attribs |= WindowAttrib::WinResizable;
        init(argc, argv);
    }

    GameApplication& GameApplication::on_init()
    {
        window.swap_interval(1);

        return *this;
    }

    GameApplication& GameApplication::on_render_frame()
    {
        static double time = Event::time();
        double new_time    = Event::time();
        static Engine::Average<double> average_fps;


        average_fps.push(1.0 / Event::diff_time());

        if (new_time - time > 0.5)
        {
            time = new_time;
            window.title(std::to_string(average_fps.average()));
            //  logger->log("FPS: %lf", average_fps.average());
            average_fps.reset();
        }

        if (KeyboardEvent::just_pressed(KEY_BACKSPACE))
        {
            auto new_status = !window.attribute(WindowAttrib::WinFullScreenDesktop);
            window.attribute(WindowAttrib::WinFullScreenDesktop, new_status);
        }

        return *this;
    }

    GameApplication::~GameApplication()
    {
        //logger->log("Max FPS: %f", _M_max_fps);
    }
}// namespace Engine


#include <string>
void test();
int game_main(int argc, char* argv[])
try
{
    std::clog << "Starting Engine" << std::endl;
    Engine::GameApplication app(argc, argv);
    app.start();
    return 0;
}
catch (const std::exception& e)
{
    Engine::logger->log("%s\n", e.what());
    return 1;
}
