#pragma once
#include <Core/application.hpp>


namespace Engine
{

    class GameApplication : public Application
    {
        float _M_max_fps = 0;
    public:
        GameApplication();
        GameApplication& on_init() override;
        GameApplication& on_render_frame() override;
        ~GameApplication();
    };
}// namespace Engine


int game_main(int argc, char* argv[]);
