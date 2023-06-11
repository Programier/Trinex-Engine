#pragma once
#include <Core/commandlet.hpp>
#include <Graphics/renderer.hpp>


namespace Engine
{
    class GameInit : public Engine::CommandLet
    {
    private:
        Engine::Renderer* _M_renderer;

    public:
        using Super = CommandLet;

        virtual int execute(int argc, char** argv) override;

        void loop();
    };
}// namespace Engine
