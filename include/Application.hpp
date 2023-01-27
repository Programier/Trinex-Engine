#pragma once
#include <Core/application.hpp>


namespace Engine
{

    class GameApplication : public Application
    {
    public:
        GameApplication();
        GameApplication& on_init() override;
    };
}// namespace Engine


int game_main(int argc, char* argv[]);
