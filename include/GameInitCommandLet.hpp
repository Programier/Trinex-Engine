#pragma once
#include <Core/commandlet.hpp>
#include <Graphics/renderer.hpp>


namespace Engine
{
    class GameInit : public Engine::CommandLet
    {
        declare_class(GameInit, CommandLet);

    public:
        virtual int execute(int argc, char** argv) override;
    };
}// namespace Engine
