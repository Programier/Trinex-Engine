#pragma once
#include <Graphics/camera.hpp>
#include <Graphics/font.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/line.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/texture_2D.hpp>
#include <Window/window.hpp>
#include <list>

namespace Engine
{

    class Application
    {
        Window window;
        Font font;
        std::wstring text = L"";
        void keyboard_procces();

    public:
        Application();
        void render();
        Application& loop();
        ~Application();
    };
}// namespace Engine


int game_main(int argc, char* argv[]);
