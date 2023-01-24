#pragma once
#include <Graphics/camera.hpp>
#include <Graphics/font.hpp>
#include <Graphics/framebuffer.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/skybox.hpp>
#include <Graphics/texture_2D.hpp>
#include <Window/window.hpp>
#include <list>
#include <Graphics/textured_object.hpp>
#include <Graphics/scene.hpp>
#include <Graphics/animator.hpp>

namespace Engine
{

    class Application
    {
        Window window;
        Font font;
        void keyboard_procces();
        Camera* camera = nullptr;
        std::vector<StaticTexturedObject*> objects;
        Scene scene;

        Animator animator;

    public:
        Application();
        void render();
        Application& loop();
        ~Application();
    };
}// namespace Engine


int game_main(int argc, char* argv[]);
