#include <GL/glew.h>
#include <Graphics/camera.hpp>
#include <Graphics/mesh.hpp>
#include <Graphics/model.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/texture.hpp>
#include <Graphics/texturearray.hpp>
#include <Window/window.hpp>
#include <glm/ext.hpp>
#include <iostream>


int main()
{
    bool light = false;

    Engine::Window window(1280, 720, "test", true);
    Engine::Shader shader(
            "Shaders/main.vert",
            "Shaders/main.frag");

    Engine::Model model;
    model.load_model("resources/de_dust2.obj");

    Engine::Camera camera(glm::vec3(321.011, 2419.2, -61.7113), glm::radians(70.f));
    camera.rotate(1.54167, 0, 3.11172);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader.use();

    while (window.is_open())
    {
        float speed = 200 * window.event.diff_time();
        window.event.poll_events();
        window.clear_buffer();
        if (window.event.keyboard.just_pressed() == Engine::KEY_TAB)
        {
            window.event.mouse.cursor_status(window.event.mouse.cursor_status() == Engine::NORMAL
                                                     ? Engine::DISABLED
                                                     : Engine::NORMAL);
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_R)
        {

            shader.load("Shaders/main.vert",
                        "Shaders/main.frag")
                    .use();
        }

        auto coords = camera.coords();
        shader.set("camera", coords);
        shader.set("light", light);
        shader.set("projview", camera.projection(window) * camera.view());
        std::string title = "x: " + std::to_string(coords.x) + ", y: " + std::to_string(coords.y) +
                            ", z: " + std::to_string(coords.z);
        window.title(title);
        const auto& offset = window.event.mouse.offset();
        if (window.event.mouse.cursor_status() == Engine::DISABLED)
            camera.rotate(offset.y / (window.height()), 0, -offset.x / (window.width()));

        if (window.event.pressed(Engine::KEY_W))
        {
            camera.move(speed, 0, 0);
        }

        if (window.event.pressed(Engine::KEY_S))
        {
            camera.move(-speed, 0, 0);
        }

        if (window.event.pressed(Engine::KEY_A))
        {
            camera.move(0, -speed, 0);
        }

        if (window.event.pressed(Engine::KEY_D))
        {
            camera.move(0.0, speed, 0);
        }

        if (window.event.keyboard.just_pressed() == Engine::KEY_C)
        {
            auto coords = camera.coords();
            std::cout << "Coords: " << coords.x << " " << coords.y << " " << coords.z << std::endl;
            auto rotation = camera.rotation();
            std::cout << "Rotation: " << rotation.x << " " << rotation.y << " " << rotation.z
                      << std::endl;
        }


        if (window.event.keyboard.just_pressed() == Engine::KEY_L)
        {
            light = !light;
        }

        model.draw();

        if (window.event.keyboard.just_pressed() == Engine::KEY_ENTER)
        {
            auto mode = window.mode();
            if (mode == Engine::NONE)
                mode = Engine::FULLSCREEN;
            else
                mode = Engine::NONE;
            window.mode(mode);
        }
        window.swap_buffers();
    }

    return 0;
}
